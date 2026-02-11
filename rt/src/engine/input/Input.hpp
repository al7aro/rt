#pragma once 

#include <map>
#include <vector>

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

namespace rt {

    /* TODO: EXTEND TO MOUSE BUTONS AND MOVEMENT */
    /* ONLY WORK FOR KEY INPUT RIGHT NOW */
    class InputHandler
    {
    protected:
        std::map<int, bool> _active_keys;
        bool _is_active;

    public:
        InputHandler(bool state = false);
        ~InputHandler();

        const bool GetIsActive() const; 
        void SetIsActive(bool active);
    };

}