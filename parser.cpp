#include <boost/json.hpp>
#include "parser.hpp"
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string.h>

namespace json = boost::json;

static json::value _parse(char const * filename)
{
	std::fstream fstream;
	fstream.open(filename, std::ios::in);

	if (!fstream.is_open())
	{
		std::cerr << "Error for opening file in reading mode !" << std::endl;
		fstream.close();
		return nullptr;
	}

	json::stream_parser parse;
	json::error_code error_code;

	std::string line;
	while (getline(fstream, line))
	{
		parse.write(line, error_code);
	}
	if (error_code)
	{
		fstream.close();
		return nullptr;
	}

	parse.finish(error_code);
	if (error_code)
	{
		fstream.close();
		return nullptr;
	}

	return parse.release();
}

static void _serialize_backup_config(std::ostream & os, json::value const & jv,
									 std::string * indent = nullptr, void * argv[] = NULL)
{
	switch (jv.kind())
	{
		case json::kind::object:
		{
			auto const & obj = jv.get_object();
			if (!obj.empty())
			{
				auto it = obj.begin();
				void * payload_pointer = NULL;
				std::string value;
				for (;;)
				{
					if (PATH_TO_DIR == it->key())
					{
						payload_pointer = (char *)argv[0];
						value = json::value_to<std::string>(it->value());

						if (payload_pointer)
							strncpy((char *)payload_pointer, value.c_str(), value.length());
						payload_pointer = NULL;
					}
					else if (PATH_TO_BACKUP == it->key())
					{
						payload_pointer = (char *)argv[1];
						value = json::value_to<std::string>(it->value());

						if (payload_pointer)
							strncpy((char *)payload_pointer, value.c_str(), value.length());
						payload_pointer = NULL;
					}
					else if (PATH_TO_LOG == it->key())
					{
						payload_pointer = (char *)argv[2];
						value = json::value_to<std::string>(it->value());

						if (payload_pointer)
							strncpy((char *)payload_pointer, value.c_str(), value.length());
						payload_pointer = NULL;
					}
					else
					{
						std::cerr << "Error for serialize data from json for backup-service"
						          << std::endl;
						return;
					}
					if(++it == obj.end())
						break;
				}
			}
			break;
		}
	}
	return;
}

static const char * __get_the_string_value(json::value const & jv)
{
	std::string returning_string;
	switch (jv.kind())
	{
		case json::kind::string:
		{
			returning_string = json::serialize(jv.get_string());
			break;
		}
		default:
		{
			return NULL;
		}
	}
	return returning_string.c_str();
}

typedef enum
{
	FW_INTERFACES_ARRAY_ATTR = 0x1,
	FW_UNTRUSTED_NET_ARRAY_ATTR,
	FW_TRUSTED_NET_ARRAY_ATTR
} fw_arrays_types_attrs_t;

static bool __traverse_array_set_values(json::value const & jv,
										fw_arrays_types_attrs_t condition,
										void ** array_settable = NULL)
{
	std::cout << "__traverse_array_set_values" << std::endl;
	int array_counter = 0;

	switch (jv.kind())
	{
		case json::kind::array:
		{
			/* `settable` is array of c-strings */
			auto const& arr = jv.get_array();
			if (!arr.empty())
			{
				auto it = arr.begin();
				for (;;)
				{
					const char * value = __get_the_string_value(*it);
					printf("value = %s\n", value);
					if (value)
					{
						printf("value = %s\n", value);
						if (FW_INTERFACES_ARRAY_ATTR == condition && SUPPORTED_INTERFACES_NUMBER > array_counter)
						{
							printf("value = %s\n", value);
							array_settable[array_counter] = (char *)malloc(SUPPORTED_INTERFACE_NAME_LENGTH * sizeof(char));
							printf("value = %s\n", value);
							strncpy((char *)array_settable[array_counter], value, SUPPORTED_INTERFACE_NAME_LENGTH);
							printf("%d: %s\n", array_counter, (char *)array_settable[array_counter]);
						}
					}
					else
					{
						std::cerr << "Unable to get string element from array" << std::endl;
						return false;
					}
					++array_counter;
					if(++it == arr.end())
						break;
				}
			}
			break;
		}
		case json::kind::object:
		default:
		{
			std::cerr << "Invalid construction of configuration file" << std::endl;
			return false;
		}
	}
	return true;
}

static void _serialize_firewall_config(std::ostream & os, json::value const & jv,
									   std::string * indent = nullptr, void * argv[] = NULL)
{
	switch (jv.kind())
	{
		case json::kind::object:
		{
			auto const& obj = jv.get_object();

			if (!obj.empty())
			{
				auto it = obj.begin();
				for (;;)
				{
					if (INTERFACE_ATTR == it->key())
					{
						char ** fw_attr_interfaces = (char **)(argv[0]);
						/* value of iterator might be array */
						if (false == __traverse_array_set_values(it->value(),
						                                         FW_INTERFACES_ARRAY_ATTR,
						                                         (void **)fw_attr_interfaces))
						{
							std::cerr << "Error for set interfaces array values" << std::endl;
							return;
						}
					}
					else if (FILTER_ATTR == it->key())
					{

					}
					else if (UNTRUSTED_NET_ATTR == it->key())
					{
					
					}
					else if (TRUSTED_NET_ATTR == it->key())
					{

					}
					else if (PACKET_NUMBERS_ATTR == it->key())
					{

					}
					else
					{
						std::cerr << "Error for serialize data from json for firewall-service"
						          << std::endl;
						return;
					}
					if(++it == obj.end())
						break;
				}
			}
			break;
		}
		case json::kind::uint64:
			os << jv.get_uint64();
			break;

		case json::kind::int64:
			os << jv.get_int64();
			break;

		case json::kind::double_:
			os << jv.get_double();
			break;

		case json::kind::bool_:
			if(jv.get_bool())
				os << "true";
			else
				os << "false";
			break;

		case json::kind::null:
			os << "null";
			break;
	}

	return;
}

int parse_and_serialize(const char * config_path, parser_condition_t type, void * argv[])
{
	auto const jv = _parse(config_path);
	if (false == jv.is_null())
	{
		switch (type)
		{
			case CONDITION_BACKUP_SERVICE:
			{
				_serialize_backup_config(std::cout, jv, nullptr, argv);
				break;
			}
			case CONDITION_FIREWALL_SERVICE:
			{
				_serialize_firewall_config(std::cout, jv, nullptr, argv);
				break;
			}
			case CONDITION_INVALID:
			default:
				return EXIT_FAILURE;
		}
	}
	else
	{
		std::cerr << "Operation of parsing is failed. Unable to parse json by path: "
		          << config_path << std::endl;
	}
	return EXIT_SUCCESS;
}

int main()
{
	backup_service_attrs_t backup_attrs = {0};
	firewall_service_attrs_t firewall_attrs = {0};

	void * backupd_argv[] = \
	{
		backup_attrs.path_to_directory,
		backup_attrs.path_to_backup,
		backup_attrs.path_to_logging,
		NULL
	};

	if (parse_and_serialize("./bin/services/backup_service/config.conf", CONDITION_BACKUP_SERVICE, backupd_argv))
	{
		std::cerr << "Process of parsing and serializing configuration is failed !"
				  << std::endl;
		return EXIT_FAILURE;
	}

	/* TEST */
	// printf("path_to_directory = %s\n", backup_attrs.path_to_directory);
	// printf("path_to_backup = %s\n", backup_attrs.path_to_backup);
	// printf("path_to_log = %s\n", backup_attrs.path_to_logging);

	void * firewalld_argv[] = \
	{
		firewall_attrs.fw_attr_interface,
		firewall_attrs.fw_attr_filter,
		firewall_attrs.fw_untrusted_network,
		firewall_attrs.fw_trusted_network,
		&firewall_attrs.fw_attr_pcts_num,
		NULL
	};
	if (parse_and_serialize("./bin/services/firewall_service/config.conf", CONDITION_FIREWALL_SERVICE, firewalld_argv))
	{
		std::cerr << "Process of parsing and serializing configuration is failed !"
		          << std::endl;
		return EXIT_FAILURE;
	}

	/* TEST */
	for (int i = 0; ; i++)
	{
		if (firewall_attrs.fw_attr_interface[i])
			printf("TEST: %s\n", firewall_attrs.fw_attr_interface[i]);
		else
			break;
	}

	return EXIT_SUCCESS;
}