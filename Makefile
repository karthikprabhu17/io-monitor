#
# Copyright (c) 2017 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.

include_dir = include

ifndef NDEBUG
CFLAGS = -g -I../$(include_dir)
else
CFLAGS = -O2 -DNDEBUG -I../$(include_dir)
endif

headers = $(include_dir)/ops.h \
          $(include_dir)/domains.h \
          $(include_dir)/ops_names.h \
          $(include_dir)/domains_names.h \
          $(include_dir)/plugin.h

plugins = plugins/sample_plugin.so \
	  plugins/output_csv.so \
	  plugins/output_table.so \
	  plugins/filter_domains.so \
          plugins/output_influxdb.so \
          plugins/input_cli.so

mq_listener_objs = mq_listener/mq_listener.o mq_listener/plugin_chain.o mq_listener/command_parser.o mq_listener/resolver.o

all: mq_listener/mq_listener io_monitor/io_monitor.so $(plugins)

#build automatic headers

$(include_dir)/ops_names.h: $(include_dir)/ops.h
	@echo -n  "generating header $@ ... "
	@cd $(include_dir) ; cat ops.h | ./enum_to_strings.sh ops_names >ops_names.h
	@echo OK

$(include_dir)/domains_names.h: $(include_dir)/domains.h
	@echo -n  "generating header $@ ... "
	@cd $(include_dir) ; cat domains.h | ./enum_to_strings.sh domains_names >domains_names.h
	@echo OK

#build io monitor
io_monitor/io_monitor.so: io_monitor/io_monitor.c $(headers) io_monitor/monitored_functions.data
	@echo -n  "generating header files for intercepts ... "
	@cd io_monitor ; ./generate_interposer.sh
	@echo OK
	@echo -n  "generating library $@ ... "
	@cd io_monitor ; gcc $(CFLAGS) -shared -fPIC io_monitor.c -o io_monitor.so -ldl -Wno-error
	@echo OK

#build listener
mq_listener/%.o: mq_listener/%.c $(headers) mq_listener/*.h
	@echo -n  "generating object $@ ... "
	@cd mq_listener ; gcc $(CFLAGS) -c ../$< -o ../$@
	@echo OK

mq_listener/%.o: mq_listener/%.cpp $(headers) mq_listener/*.h
	@echo -n  "generating object $@ ... "
	@cd mq_listener ; g++ $(CFLAGS) -c ../$< -o ../$@
	@echo OK

mq_listener/mq_listener: $(mq_listener_objs)
	@echo -n  "generating executable $@ ... "
	@g++ $(CFLAGS) $^ -o mq_listener/mq_listener -lcurl -ldl -lpthread
	@echo OK

#build sample plugin
plugins/input_cli.so: plugins/input_cli.c $(headers)
	@echo -n  "generating plugin $@ ... (L) "
	@cd plugins ; gcc $(CFLAGS) -lpthread -shared -fPIC ../$< -o ../$@
	@echo OK

plugins/output_influxdb.so: plugins/output_influxdb.c $(headers)
	@echo -n  "generating plugin $@ ... (L) "
	@cd plugins ; gcc $(CFLAGS) -lpthread -shared -fPIC ../$< -o ../$@
	@echo OK

plugins/%.so: plugins/%.c $(headers)
	@echo -n  "generating plugin $@ ... "
	@cd plugins ; gcc $(CFLAGS) -shared -fPIC ../$< -o ../$@
	@echo OK



clean:
	rm -f mq_listener/mq_listener
	rm -f io_monitor/io_monitor.so
	rm -f $(include_dir)/domains_names.h
	rm -f $(include_dir)/ops_names.h
	rm -f io_monitor/io_function_types.h
	rm -f io_monitor/io_function_orig_handlers.h
	rm -f io_monitor/intercept_functions.h
	rm -f io_monitor/assign_functions.h
	rm -f plugins/*.so

