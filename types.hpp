#pragma once
#include <vector>

struct GridConnection{
    typedef unsigned UUID;
    GridConnection() : uuid(0) {

    }

    GridConnection(UUID uuid) : uuid(uuid) {
    
    }
    UUID uuid;
};

struct GridProperty{
    typedef unsigned UUID;
    GridProperty() : uuid(0){
    
    }

    GridProperty(UUID uuid) : uuid(uuid){

    }

    UUID uuid;

};





