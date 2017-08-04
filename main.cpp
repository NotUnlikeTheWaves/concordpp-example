#include "concordpp.h"
#include <json.hpp>
#include <iostream>
#include <fstream>
#include <stdio.h>

concordpp::rest::rest_client *d_rest;
concordpp::gateway::gateway_client *d_gateway;

    // This is older code, as an example of event callbacks.
    // I suggest using commands instead for command purposes.
void on_message(nlohmann::json data) {
    std::string content = data["content"];
    if(content == "test me") {
        d_rest->create_message(data["channel_id"], "This is (not) a test");
    } else if(content == "shutdown") {
        d_rest->create_message(data["channel_id"], "zzz", [](int code, nlohmann::json data) {
                d_gateway->stop();
        });
    } else if(content == "channels") {
        std::string tempchan = data["channel_id"];
        std::string *channel = new std::string(tempchan);
        d_rest->get_current_user_guilds([channel](int code, nlohmann::json data){
            int *count = new int(data.size());
            std::string *response = new std::string("");
            for (nlohmann::json::iterator it = data.begin(); it != data.end(); ++it) {
                d_rest->get_guild_channels((*it)["id"], [channel, count, response] (int code, nlohmann::json data) {
                    (*count) = (*count) - 1;
                    for(nlohmann::json::iterator it = data.begin(); it != data.end(); ++it) {
                        std::string chan_name = (*it)["name"];
                        (*response) = (*response) + chan_name + "\n";
                    }
                    if((*count) == 0) {
                        d_rest->create_message((*channel), "```json\n" + (*response) + "\n```", [](int code, nlohmann::json data) {
                        });
                        delete channel;
                        delete response;
                        delete count;
                    }
                });
            }
        });
    }
}

    // Read the token file.
std::string read_token(std::string token_file) {
    std::string token;
    std::ifstream files;
    files.open(token_file);
    if(files.is_open()) {
        std::getline(files, token);
    } else {
        std::cout << "error opening token file, exiting" << std::endl;
        exit(1);
    }
    files.close();
    return token;
}

    // Attempt to get a neofetch or screenfetch.
std::string get_screenfetch() {
	FILE *in;
	char buff[512];
        // Try screenfetch first.
	if(!(in = popen("neofetch | sed 's/\x1B[[0-9;?]*[a-zA-Z]//g'", "r"))) {
        return "Could not run neofetch or screenfetch.";
	}
	std::string fetch = "";
	while(fgets(buff, sizeof buff, in) != NULL) {
        fetch = fetch + buff;
    }
    pclose(in);
	return fetch;
}

int main(int argc, char* argv[]) {
    std::string token = read_token("./token.txt");
    concordpp::debug::set_log_level(concordpp::debug::log_level::INFORMATIONAL);
    d_gateway = new concordpp::gateway::gateway_client(token);
    d_rest = new concordpp::rest::rest_client(token);
    d_gateway->add_callback("MESSAGE_CREATE", on_message);
    d_gateway->add_command("neofetch", [](nlohmann::json data){
        std::string fetch = get_screenfetch();
        if(fetch == "") fetch = "Could not run screenfetch.";
        else std::replace(fetch.begin(), fetch.end(), '`', '\'');
        d_rest->create_message(data["channel_id"], "```\n" + fetch + "\n```");
    });
    d_gateway->add_command("set-game", [](nlohmann::json data) {
        d_gateway->set_status(concordpp::gateway::status_types::DO_NOT_DISTURB, data["content"]);
    });
    d_gateway->add_command("echo", [](nlohmann::json data) {
        std::string response = data["content"];
        d_rest->create_message(data["channel_id"], "You typed: " + response);
    });
    d_gateway->connect();   // Blocking call
    delete d_gateway;
    delete d_rest;
}
