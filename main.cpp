#include "concordpp.h"
#include <json.hpp>
#include <iostream>
#include <fstream>
#include <ctime>

int quit = 0;
concordpp::rest::rest_client *d_rest;
concordpp::gateway::gateway_client *d_gateway;

void on_message(nlohmann::json data) {
    if(data["content"] == "test me") {
        d_rest->create_message(data["channel_id"], "This is (not) a test");
    } else if(data["content"] == "shutdown") {
        d_rest->create_message(data["channel_id"], "zzz", [](int code, nlohmann::json data) {
                d_gateway->stop();
        });
    } else if(data["content"] == "channels") {
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

int main(int argc, char* argv[]) {
    std::string token = read_token("./token.txt");
    concordpp::debug::set_log_level(concordpp::debug::log_level::INFORMATIONAL);
    d_gateway = new concordpp::gateway::gateway_client(token);
    d_rest = new concordpp::rest::rest_client(token);
    d_gateway->add_callback("MESSAGE_CREATE", on_message);
    d_gateway->add_callback("MESSAGE_CREATE", [](nlohmann::json data){
        if(data["content"] == "lambda") {
            d_rest->create_message(data["channel_id"], "Lambda test done.");
        }
    });
    d_gateway->add_callback("MESSAGE_CREATE", [](nlohmann::json data) {
        std::string arg = data["content"];
        std::string cmd = arg.substr(0, arg.find(' '));
        if(cmd != arg && cmd == "game") {
            std::string remainder = arg.substr(arg.find(' ') + 1, std::string::npos);
            d_gateway->set_status(concordpp::gateway::status_types::DO_NOT_DISTURB, remainder);
        }
    });
    d_gateway->connect();   // Blocking call
    delete d_gateway;
    delete d_rest;
}
