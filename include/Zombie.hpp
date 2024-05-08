#pragma once

#include <Headers.hpp>

class Transform; // Déclaration anticipée

struct Node{
    std::vector<unsigned short> indices; // Indices des triangles concaténés dans une liste
    std::vector<glm::vec3> indexed_vertices;
    std::vector<Node*> fils; // Ensemble des fils de l'objet du noeud
    // Pas besoin de coordonnées uv, elles seront déterminées dans les shaders

    GLuint vertexbuffer;
    GLuint elementbuffer;

    int nodeID;
    Transform *transformation;
    glm::vec3 center;
};

class Zombie{
    private:
        Node *node;
        float speedEntity;
    public:
        Zombie(int ID,glm::vec3 pos, float speedEntity);
        ~Zombie();
        void loadZombie();
        void drawZombie(GLuint programID_Entity);

        void loadBufferNode(Node *node);
        void sendNodeToShader(Node *node,GLuint programID_Entity,glm::mat4 parent);

        void setPave(Node* node, glm::vec3 dimensions, glm::vec3 position);

        void createZombie(Node* node, glm::vec3 position);
        
        void walk(Node* node,float angle, float deltaTime);
        void reset(Node* node);
        void attack(Node* node, bool *attack, float *accumulateurAnimation, float deltaTime);
        void die(Node* node, bool *die, float *accumulateurAnimation, float deltaTime);
            
        Node* getRootNode();
};