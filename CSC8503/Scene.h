﻿
#pragma once
#include <memory>
#include "../NCLCoreClasses/KeyboardMouseController.h"
namespace NCL{
    namespace CSC8503{
        class LevelManager;

        class Scene{
        public:
            Scene();
            ~Scene();

            virtual void UpdateGame(float dt);

            std::unique_ptr<LevelManager> GetLevelManager();

        protected:
            std::unique_ptr<LevelManager> mLevelManager;

            virtual void InitCamera();

            KeyboardMouseController mController;
        };
    }
}