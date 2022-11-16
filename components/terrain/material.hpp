#ifndef COMPONENTS_TERRAIN_MATERIAL_H
#define COMPONENTS_TERRAIN_MATERIAL_H

#include <vector>

#include <osg/Texture2D>
#include <osg/ref_ptr>

namespace osg
{
    class StateSet;
}

namespace Resource
{
    class SceneManager;
}

namespace Terrain
{

    struct TextureLayer
    {
        osg::ref_ptr<osg::Texture2D> mDiffuseMap;
        osg::ref_ptr<osg::Texture2D> mNormalMap; // optional
        bool mParallax;
        bool mSpecular;
    };

    std::vector<osg::ref_ptr<osg::StateSet>> createPasses(bool useShaders, Resource::SceneManager* sceneManager,
        const std::vector<TextureLayer>& layers, const std::vector<osg::ref_ptr<osg::Texture2D>>& blendmaps,
        int blendmapScale, float layerTileSize);

}

#endif
