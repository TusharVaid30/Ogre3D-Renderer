#include "Ogre.h"
#include "OgreApplicationContext.h"
#include <iostream>
#include <OgreWindowEventUtilities.h>
#include <cstdlib>

bool move = false;
bool moved = false;
Ogre::SceneNode* selectedNode;
Ogre::Vector2 *startMousePosition;

float cubeScale = 0.3;
float shift = 0.7;

Ogre::SceneNode* allNodes[2];
Ogre::Vector3 nodesPositions[2] = {Ogre::Vector3(0, 0.7, 0), 
    Ogre::Vector3(0, 0, 0)};

float lerp(float a, float b, float f)
{
    return a * (1.0 - f) + (b * f);
}

double clamp(double x, double upper, double lower)
{
    return std::min(upper, std::max(x, lower));
}

bool CheckTile(Ogre::SceneManager* scnMgr, Ogre::Vector3 pos, Ogre::Vector3 direction)
{
    Ogre::Ray rayCast(pos + (direction * 0.4), direction);

    Ogre::RaySceneQuery* rq = scnMgr->createRayQuery(rayCast);
    
    rq->setSortByDistance(true, 1);
    rq->setQueryTypeMask(Ogre::SceneManager::ENTITY_TYPE_MASK);

    Ogre::RaySceneQueryResult res = rq->execute();
    Ogre::RaySceneQueryResult::iterator it = res.begin();

    if (it != res.end())
    {
        if ((pos + (direction * 0.4)).squaredDistance
        (it->movable->getParentSceneNode()->getPosition()) < Ogre::Real(0.2f))
        {
            std::cout << "tileFound";

            return true;
        }
    }

    return false;
}

class KeyHandler : public OgreBites::InputListener
{
private:
    Ogre::SceneNode* player;
    Ogre::Camera* playerCam;
    Ogre::SceneManager* scene;

public:
    KeyHandler(Ogre::SceneNode* node, Ogre::Camera* cam, Ogre::SceneManager* scnMgr)
    {
        player = node;
        playerCam = cam;
        scene = scnMgr;
    }

    bool mousePressed(const OgreBites::MouseButtonEvent& evt)
    {
        startMousePosition = new Ogre::Vector2(evt.x, evt.y);

        std::cout << startMousePosition->x << " " << startMousePosition->y << std::endl;

        float width = (float)this->playerCam->getViewport()->getActualWidth(); // viewport width
        float height = (float)this->playerCam->getViewport()->getActualHeight(); // viewport height

        Ogre::Ray ray = this->playerCam->getCameraToViewportRay((float)evt.x / width, (float)evt.y / height);

        // Set up the ray query - you will probably not want to create this every time
        Ogre::RaySceneQuery* rq = this->scene->createRayQuery(ray);

        // Sort by distance, and say we're only interested in the first hit; also, only pick entities
        rq->setSortByDistance(true, 1);
        rq->setQueryTypeMask(Ogre::SceneManager::ENTITY_TYPE_MASK);

        // Execute
        Ogre::RaySceneQueryResult res = rq->execute();
        Ogre::RaySceneQueryResult::iterator it = res.begin();

        if (it != res.end())
        {
            selectedNode = it->movable->getParentSceneNode();
        }

        this->scene->destroyQuery(rq);

        return true;
    }

    bool mouseReleased(const OgreBites::MouseButtonEvent& evt)
    {
        selectedNode = nullptr;
        moved = false;
        return true;
    }

    bool mouseMoved(const OgreBites::MouseMotionEvent& evt)
    {
        if (selectedNode != nullptr)
        {
            move = true;

            Ogre::Vector2* currentlocation = new Ogre::Vector2(evt.x, evt.y);

            Ogre::Vector2 direction((currentlocation->x - startMousePosition->x),
                (currentlocation->y - startMousePosition->y));

            if (std::abs(direction.x) > std::abs(direction.y) && !moved)
            {
                moved = true;
                if (direction.x < 0)
                {
                    for (int i = 0; i < 2; i++)
                    {
                        if (allNodes[i]->getName() == selectedNode->getName())
                        {
                            if (!CheckTile(scene, selectedNode->getPosition(), Ogre::Vector3(-1, 0, 0)))
                                nodesPositions[i].x -= shift;
                        }
                    }
                }
                if (direction.x > 0)
                {
                    for (int i = 0; i < 2; i++)
                    {
                        if (allNodes[i]->getName() == selectedNode->getName())
                            if (!CheckTile(scene, selectedNode->getPosition(), Ogre::Vector3(1, 0, 0)))
                                nodesPositions[i].x += shift;
                    }
                }
            }
            if (std::abs(direction.x) < std::abs(direction.y) && !moved)
            {
                moved = true;
                if (direction.y < 0)
                {
                    for (int i = 0; i < 2; i++)
                    {
                        if (allNodes[i]->getName() == selectedNode->getName())
                            if (!CheckTile(scene, selectedNode->getPosition(), Ogre::Vector3(0, 1, 0)))
                                nodesPositions[i].y += shift;
                    }
                }
                if (direction.y > 0)
                {
                    for (int i = 0; i < 2; i++)
                    {
                        if (allNodes[i]->getName() == selectedNode->getName())
                            if (!CheckTile(scene, selectedNode->getPosition(), Ogre::Vector3(0, -1, 0)))
                                nodesPositions[i].y -= shift;
                    }
                }
            }
        }

        return true;
    }
};

class Frames : public Ogre::FrameListener
{
    bool frameStarted(const Ogre::FrameEvent& evt)
    {
        for (int i = 0; i < 2; i++)
        {
            Ogre::Vector3 pos = allNodes[i]->getPosition();

            nodesPositions[i].x = clamp(nodesPositions[i].x, 2.1, -2.1);
            nodesPositions[i].y = clamp(nodesPositions[i].y, 1.4, -1.4);

            pos.x = lerp(pos.x, nodesPositions[i].x, 
                10.0f * evt.timeSinceLastFrame);
            pos.y = lerp(pos.y, nodesPositions[i].y,
                10.0f * evt.timeSinceLastFrame);
            allNodes[i]->setPosition(pos);
        }

        return true;
    }
};

int main(int argc, char* argv[])
{
    OgreBites::ApplicationContext ctx("OgreTutorialApp");
    ctx.initApp();

    Ogre::Root* root = ctx.getRoot();
    Ogre::SceneManager* scnMgr = root->createSceneManager();

    // register our scene with the RTSS
    Ogre::RTShader::ShaderGenerator* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
    shadergen->addSceneManager(scnMgr);

    scnMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

    // without light we would just get a black screen    
    Ogre::Light* light = scnMgr->createLight("MainLight");
    Ogre::SceneNode* lightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    lightNode->setPosition(-160, 0, 0);
    light->setDiffuseColour(Ogre::ColourValue(1.0, 1.0, 1.0));
    lightNode->attachObject(light);

    // also need to tell where we are
    Ogre::SceneNode* camNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    camNode->setPosition(0, 1, 5);
    camNode->lookAt(Ogre::Vector3(0, 0, -1), Ogre::Node::TS_PARENT);

    // create the camera
    Ogre::Camera* cam = scnMgr->createCamera("myCam");
    cam->setNearClipDistance(0.01); // specific to this sample
    cam->setAutoAspectRatio(true);
    camNode->attachObject(cam);

    // and tell it to render into the main window
    ctx.getRenderWindow()->addViewport(cam)->setBackgroundColour(Ogre::ColourValue(1, 1, 1));

    // finally something to render
    Ogre::Entity* ent = scnMgr->createEntity("Textured.mesh");
    ent->setCastShadows(true);
    ent->setMaterialName("BrickWall2");
    Ogre::Entity* ent2 = scnMgr->createEntity("Colored.mesh");

    Ogre::SceneNode* node = scnMgr->getRootSceneNode()->createChildSceneNode("cube1");
    node->attachObject(ent);
    node->setPosition(0, 0, 0);
    node->pitch(Ogre::Degree(90.0f));
    node->setScale(Ogre::Vector3(cubeScale, cubeScale, cubeScale));

    Ogre::SceneNode* node2 = scnMgr->getRootSceneNode()->createChildSceneNode("cube2");
    node2->attachObject(ent2);
    node2->setPosition(1, 0, 0);
    node2->pitch(Ogre::Degree(90.0f));
    node2->setScale(Ogre::Vector3(cubeScale, cubeScale, cubeScale));

    allNodes[0] = node;
    allNodes[1] = node2;
    
    KeyHandler keyHandler(node, cam, scnMgr);
    ctx.addInputListener(&keyHandler);

    Frames frame;
    ctx.getRoot()->addFrameListener(&frame);
    ctx.getRoot()->startRendering();

    ctx.closeApp();
    return 0;
}
