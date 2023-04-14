#include "Ogre.h"
#include "OgreApplicationContext.h"
#include <iostream>
#include <OgreWindowEventUtilities.h>

bool move = false;

class WindowHandler : public OgreBites::ApplicationContextBase
{
    void windowClosed(Ogre::RenderWindow* rw) override
    {
        std::cout << "cloose";
    }
};

class KeyHandler : public OgreBites::InputListener
{
private:
    Ogre::SceneNode* player;

public:
    KeyHandler(Ogre::SceneNode* node)
    {
        player = node;
    }

    bool keyPressed(const OgreBites::KeyboardEvent& evt) override
    {
        if (evt.keysym.sym == OgreBites::SDLK_LEFT)
        {
            move = true;
        }
        return true;
    }
    bool keyReleased(const OgreBites::KeyboardEvent& evt) override
    {
        if (evt.keysym.sym == OgreBites::SDLK_LEFT)
        {
            move = false;
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
    Ogre::Entity* ent = scnMgr->createEntity("TESTTEST.mesh");
    ent->setCastShadows(true);
    ent->setMaterialName("Examples/Rockwall");

    Ogre::SceneNode* node = scnMgr->getRootSceneNode()->createChildSceneNode();
    node->attachObject(ent);
    node->setPosition(0, 0, 0);
    node->pitch(Ogre::Degree(-90));
    
    // register for input events
    KeyHandler keyHandler(node);
    ctx.addInputListener(&keyHandler);

    WindowHandler wh;

    while (ctx.getRoot()->renderOneFrame())
    {
        Ogre::WindowEventUtilities::messagePump();
        if (move)
            node->translate(-0.01, 0, 0);
    }

    ctx.closeApp();
    return 0;
}
