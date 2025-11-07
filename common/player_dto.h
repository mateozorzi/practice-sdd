#ifndef PLAYER_DTO_H
#define PLAYER_DTO_H


#include <string>


struct PlayerDTO {


    std::string username;


    explicit PlayerDTO(const std::string& username);
};


#endif