#pragma once

#include <Headers.hpp>

struct Mouvement{
    float timeMotion;
    glm::vec3 direction;
    float angleForLeg;
    float angleViewToReach;
};

class Agent{
    private :
        bool isMoving;
        Mouvement mouvement;
        float angleOfView;
    public :
        Agent();
        ~Agent();
        void resetMouvement();
        void createMouvement();
        bool getIsMoving();
        float getRemainingTime();
        void setIsMoving(bool isMoving);
        void addToAngleForLeg(float angleForLeg);
        float getAngleForLeg();
        void timePass(float timeMotion);
        glm::vec3 getDirection();
};