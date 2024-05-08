#include <Hitbox.hpp>

Hitbox::Hitbox(glm::vec3 position, float gravity, float forceJumpInitial){
    this->bottomPoint = glm::vec3(position + glm::vec3(0.5f,0.0f,0.5f));
    this->canJump = false;
    this->gravity = gravity;
    this->forceJump = 0.0f;
    this->forceJumpInitial = forceJumpInitial;
}

Hitbox::~Hitbox(){
    std::cout << "Destructeur de Hitbox\n";
}

glm::vec3 Hitbox::getBottomPoint(){
    return this->bottomPoint;
}

void Hitbox::move(glm::vec3 motion){
    this->bottomPoint += motion;
}

bool Hitbox::getCanJump(){
    return this->canJump;
}

void Hitbox::setCanJump(bool canJump){
    this->canJump = canJump;
}

void Hitbox::checkJump(bool *hasUpdate, float deltaTime){
    if (!this->getCanJump()){ // Le joueur est en train de sauter
        this->move(glm::vec3(0.0,this->forceJump*deltaTime,0.0));
        this->forceJump -= this->gravity*deltaTime;
        *hasUpdate = true;
    }
}

void Hitbox::resetJumpForce(){
    this->forceJump = this->forceJumpInitial;
}

// axisToCheck --> true pour Left/Right, false pour Front/Back
bool Hitbox::getLateralMovePossible(bool axisToCheck,float directionCheck, glm::vec3 bottomPlayer, glm::vec3 camera_target, glm::vec3 camera_up, int planeWidth, int planeLength, std::vector<Chunk*> listeChunks,glm::vec3 *cross_point){
    std::vector<glm::vec3> points; // On crée les 3 points qui serviront à la détection de la collision
    if (axisToCheck){
        *cross_point = glm::normalize(glm::cross(camera_target,camera_up))*directionCheck;
    }else{
        glm::vec3 ct = camera_target;
        ct[1] = 0.0f;
        *cross_point = glm::normalize(ct)*directionCheck;
    }
    glm::vec3 collision_point = bottomPlayer+*cross_point;
    for (int i = 0 ; i < 3 ; i++){
        points.push_back(collision_point);
        collision_point[1] += 0.9;
    }

    bool canMove = true;
    for (int i = 0 ; i < 3 ; i++){ // On regarde si l'un des points cause une collision
        int NL = floor(points[i][0]) + 16*planeWidth;
        int NH = floor(points[i][1]) + 16;
        int NP = floor(points[i][2]) + 16*planeLength;
        // Il ne faut pas oublier de vérifier si ce point se trouve bien dans le chunk
        if (!(NL < 0 || NL > (planeWidth*32)-1 || NP < 0 || NP > (planeLength*32)-1 || NH < 0 || NH > 31)){
            int index = NH *1024 + (NP%32) * 32 + (NL%32); // Indice du voxel dans lequel on considère que le point se trouve
            Voxel *v = listeChunks[(NL/32) * planeLength + NP/32]->getListeVoxels()[index];
            if (v != nullptr){ // Il y a une collision qui est détectée
                canMove = false;
                break; // Inutile de regarder pour les autres points
            }
        }
    }
    return canMove;
}

void Hitbox::checkTopAndBottomCollision(bool hasUpdate, int planeWidth, int planeLength, float deltaTime, std::vector<Chunk*> listeChunks, Hud *hud, Player *player){
    // Détermine la cellule ou se trouve le joueur
    glm::vec3 pPlayer = this->getBottomPoint();
    
    int numLongueur = floor(pPlayer[0]) + 16*planeWidth;
    int numHauteur = floor(pPlayer[1]-0.001) + 16;
    int numProfondeur = floor(pPlayer[2]) + 16*planeLength;
    if (numLongueur < 0 || numLongueur > (planeWidth*32)-1 || numProfondeur < 0 || numProfondeur > (planeLength*32)-1 || numHauteur < 0 || numHauteur > 31){
        // C'est contre_intuitif, mais on ne peut pas juste vérifier si le joueur a déjà sauté.
        // On est obligé d'utiliser un nouveau booléen si on veut l'empêcher de sauter + ne pas le faire subir la gravité 2 fois
        if (!hasUpdate){
            this->move(glm::vec3(0.0,this->forceJump*deltaTime,0.0));
            this->forceJump -= this->gravity*deltaTime;
            this->setCanJump(false);
        }
    }else{
        int indiceBlock = numHauteur *1024 + (numProfondeur%32) * 32 + (numLongueur%32); // Indice du voxel dans lequel on considère que le joueur se trouve
        Voxel *v = listeChunks[(numLongueur/32) * planeLength + numProfondeur/32]->getListeVoxels()[indiceBlock];
    
        if (!this->getCanJump()){ // Collision vers le haut (temporaire, il faudra faire plus propre)
            glm::vec3 pPlayerTop = pPlayer;
            pPlayerTop[1] += 1.8; // Attention à bien mettre une hitbox plus haute que la position de la caméra (sinon on peut voir parfois à travers des blocs)
            int NL = floor(pPlayerTop[0]) + 16*planeWidth;
            int NH = floor(pPlayerTop[1]) + 16;
            int NP = floor(pPlayerTop[2]) + 16*planeLength;
            // Il ne faut pas oublier de vérifier si le point au sommet de la hitbox du joueur se trouve bien dans le chunk
            if (!(NL < 0 || NL > (planeWidth*32)-1 || NP < 0 || NP > (planeLength*32)-1 || NH < 0 || NH > 31)){
                int index = NH *1024 + (NP%32) * 32 + (NL%32); // Indice du voxel dans lequel on considère que le joueur se trouve
                Voxel *vTop = listeChunks[(NL/32) * planeLength + NP/32]->getListeVoxels()[index];
                if (vTop != nullptr){
                    this->forceJump = 0.0f;
                    this->move(glm::vec3(0.0f,floor(pPlayerTop[1]) - pPlayerTop[1] - 0.01,0.0f)); // Le 0.01 assure que le joueur ne rentre pas dans le bloc
                }
            }
        }

        if (v == nullptr){
            this->setCanJump(false);
            if (!hasUpdate){ // Pour ne pas faire subir 2 fois la gravité au joueur
                this->move(glm::vec3(0.0,this->forceJump*deltaTime,0.0));
                this->forceJump -= this->gravity*deltaTime;
            }
            
            // On fait attention à ne pas afficher le joueur à l'intérieur d'un bloc (même pendant une frame, c'est plus propre)
            pPlayer = this->getBottomPoint();
            numHauteur = floor(pPlayer[1]-0.001) + 16;
            indiceBlock = numHauteur *1024 + (numProfondeur%32) * 32 + (numLongueur%32);
            v = listeChunks[(numLongueur/32) * planeLength + numProfondeur/32]->getListeVoxels()[indiceBlock];
            if (v != nullptr){
                if(v->getID()==33 && this->forceJump<-0.1){
                    this->forceJump*=-0.5;

                }else{
                    this->move(glm::vec3(0.f,ceil(pPlayer[1]) - pPlayer[1],0.f));
                    if (this->forceJump <= -14.0f){
                        player->takeDamage(pow(this->forceJump + 14.0, 2)); // On reverra plus le calcul des dégâts si on a le temps
                        hud->updateLife(player->getLife());
                        if (player->getLife() <= 0.0){
                            std::cout << "Vous êtes mort !\n";
                            //return -1; Le joueur est mort, le programme s'arrête 
                        }
                    }
                    this->forceJump = 0.0f;
                    this->setCanJump(true);
                }
            }
        }else{
            if(v->getID()==33 && this->forceJump<-0.1){
                this->forceJump*=-0.5;
            }else{
                if (pPlayer[1] != ceil(pPlayer[1])){
                    this->move(glm::vec3(0.f,ceil(pPlayer[1]) - pPlayer[1],0.f));
                    if (this->forceJump <= -14.0f){
                        player->takeDamage(pow(this->forceJump + 14.0, 2)); // On reverra plus le calcul des dégâts si on a le temps
                        hud->updateLife(player->getLife());
                        if (player->getLife() <= 0.0){
                            std::cout << "Vous êtes mort !\n";
                            //return -1; Le joueur est mort, le programme s'arrête 
                        }
                    }
                    this->forceJump = 0.0f;
                }
                this->setCanJump(true);
            }
        }
    }
}