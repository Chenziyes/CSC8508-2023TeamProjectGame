#include "LevelManager.h"
#include "GameWorld.h"
#include "RecastBuilder.h"

#include "PlayerObject.h"
#include "GuardObject.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "AnimationObject.h"
#include <filesystem>

using namespace NCL::CSC8503;

LevelManager::LevelManager() {
	mBuilder = new RecastBuilder();
	mWorld = new GameWorld();
	mRenderer = new GameTechRenderer(*mWorld);
	mPhysics = new PhysicsSystem(*mWorld);
	mPhysics->UseGravity(true);
	mAnimation = new AnimationSystem(*mWorld);

	mRoomList = std::vector<Room*>();
	for (const auto& entry : std::filesystem::directory_iterator("../Assets/Levels/Rooms")) {
		Room* newRoom = new Room(entry.path().string());
		mRoomList.push_back(newRoom);
	}
	mLevelList = std::vector<Level*>();
	for (const auto& entry : std::filesystem::directory_iterator("../Assets/Levels/Levels")) {
		Level* newLevel = new Level(entry.path().string());
		mLevelList.push_back(newLevel);
	}
	mActiveLevel = -1;

	InitialiseAssets();
}

LevelManager::~LevelManager() {
	for (int i = 0; i < mRoomList.size(); i++) {
		delete(mRoomList[i]);
	}
	mRoomList.clear();
	for (int i = 0; i < mLevelList.size(); i++) {
		delete(mLevelList[i]);
	}
	mLevelList.clear();
	for (int i = 0; i < mLevelLayout.size(); i++) {
		delete(mLevelLayout[i]);
	}
	mLevelLayout.clear();
	for (int i = 0; i < mUpdatableObjects.size(); i++) {
		delete(mUpdatableObjects[i]);
	}
	mUpdatableObjects.clear();

	delete mTempPlayer;

	delete mCubeMesh;
	delete mSphereMesh;
	delete mCapsuleMesh;
	delete mCharMesh;
	delete mEnemyMesh;
	delete mBonusMesh;
	delete mSoldierAnimation;
	delete mSoldierMaterial;
	delete mSoldierMesh;
	delete mSoldierShader;

	delete mBasicTex;
	delete mBasicShader;
	delete mKeeperAlbedo;
	delete mKeeperNormal;
	delete mFloorAlbedo;
	delete mFloorNormal;

	delete mPhysics;
	delete mRenderer;
	delete mWorld;
	delete mAnimation;
}

void LevelManager::LoadLevel(int levelID, int playerID) {
	if (levelID > mLevelList.size() - 1) return;
	mWorld->ClearAndErase();
	mPhysics->Clear();
	for (int i = 0; i < mLevelLayout.size(); i++) {
		delete(mLevelLayout[i]);
	}
	mLevelLayout.clear();
	LoadMap((*mLevelList[levelID]).GetTileMap(), Vector3(0, 0, 0));
	LoadLights((*mLevelList[levelID]).GetLights(), Vector3(0, 0, 0));
	for (auto const& [key, val] : (*mLevelList[levelID]).GetRooms()) {
		switch ((*val).GetType()) {
		case Medium:
			for (Room* room : mRoomList) {
				if (room->GetType() == Medium) {
					LoadMap(room->GetTileMap(), key);
					LoadLights(room->GetLights(), key);
					break;
				}
			}
			break;
		}
	}
	mBuilder->BuildNavMesh(mLevelLayout);
	AddPlayerToWorld((*mLevelList[levelID]).GetPlayerStartPosition(playerID) * 10, "Player");
	mActiveLevel = levelID;
}

void LevelManager::Update(float dt, bool isUpdatingObjects) {
	if ((mUpdatableObjects.size() > 0) && isUpdatingObjects) {
		for (GameObject* obj : mUpdatableObjects) {
			obj->UpdateObject(dt);
		}
	}

	mWorld->UpdateWorld(dt);
	mRenderer->Update(dt);
	mPhysics->Update(dt);
	mAnimation->Update(dt);
	mRenderer->Render();
	Debug::UpdateRenderables(dt);
}

void LevelManager::InitialiseAssets() {
	mCubeMesh = mRenderer->LoadMesh("cube.msh");
	mSphereMesh = mRenderer->LoadMesh("sphere.msh");
	mCapsuleMesh = mRenderer->LoadMesh("Capsule.msh");
	mCharMesh = mRenderer->LoadMesh("goat.msh");
	mEnemyMesh = mRenderer->LoadMesh("Keeper.msh");
	mBonusMesh = mRenderer->LoadMesh("apple.msh");
	mCapsuleMesh = mRenderer->LoadMesh("capsule.msh");

	mBasicTex = mRenderer->LoadTexture("checkerboard.png");
	mKeeperAlbedo = mRenderer->LoadTexture("fleshy_albedo.png");
	mKeeperNormal = mRenderer->LoadTexture("fleshy_normal.png");
	mFloorAlbedo = mRenderer->LoadTexture("panel_albedo.png");
	mFloorNormal = mRenderer->LoadTexture("panel_normal.png");

	mBasicShader = mRenderer->LoadShader("scene.vert", "scene.frag");

	mSoldierMesh = mRenderer->LoadMesh("Role_T.msh");
	mSoldierAnimation = mRenderer->LoadAnimation("Role_T.anm");
	mSoldierMaterial = mRenderer->LoadMaterial("Role_T.mat");
	mSoldierShader = mRenderer->LoadShader("SkinningVertex.glsl", "scene.frag");
}

void LevelManager::LoadMap(const std::map<Vector3, TileType>& tileMap, const Vector3& startPosition) {
	for (auto const& [key, val] : tileMap) {
		switch (val) {
		case Wall:
			AddWallToWorld(key + startPosition);
			break;
		case Floor:
			AddFloorToWorld(key + startPosition);
			break;
		}
	}
}

void LevelManager::LoadLights(const std::vector<Light*>& lights, const Vector3& centre) {
	for (int i = 0; i < lights.size(); i++) {
		auto* pl = dynamic_cast<PointLight*>(lights[i]);
		if (pl) {
			pl->SetPosition((pl->GetPosition() + centre) * 10);
			pl->SetRadius(pl->GetRadius() * 10);
			mRenderer->AddLight(pl);
			continue;
		}
		auto* sl = dynamic_cast<SpotLight*>(lights[i]);
		if (sl) {
			sl->SetPosition((pl->GetPosition() + centre) * 10);
			sl->SetRadius(sl->GetRadius() * 10);
			mRenderer->AddLight(sl);
			continue;
		}
		mRenderer->AddLight(lights[i]);
	}
}

GameObject* LevelManager::AddWallToWorld(const Vector3& position) {
	GameObject* wall = new GameObject(StaticObj, "Wall");

	Vector3 wallSize = Vector3(5, 5, 5);
	AABBVolume* volume = new AABBVolume(wallSize);
	wall->SetBoundingVolume((CollisionVolume*)volume);
	wall->GetTransform()
		.SetScale(wallSize * 2)
		.SetPosition(position*10);

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), mCubeMesh, mFloorAlbedo, mFloorNormal, mBasicShader, 1));
	wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

	wall->GetPhysicsObject()->SetInverseMass(0);
	wall->GetPhysicsObject()->InitCubeInertia();

	wall->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	mWorld->AddGameObject(wall);

	mLevelLayout.push_back(wall);

	return wall;
}

GameObject* LevelManager::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject(StaticObj, "Floor");

	Vector3 wallSize = Vector3(5, 0.5f, 5);
	AABBVolume* volume = new AABBVolume(wallSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(wallSize * 2)
		.SetPosition(position*10);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), mCubeMesh, mFloorAlbedo, mFloorNormal, mBasicShader, 1));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	floor->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 1));

	mWorld->AddGameObject(floor);

	mLevelLayout.push_back(floor);

	return floor;
}

PlayerObject* LevelManager::AddPlayerToWorld(const Vector3 position, const std::string& playerName) {
	mTempPlayer = new PlayerObject(mWorld, playerName);
	CreatePlayerObjectComponents(*mTempPlayer, position);

	mWorld->AddGameObject(mTempPlayer);
	mUpdatableObjects.push_back(mTempPlayer);
	mTempPlayer->SetActive();
	return mTempPlayer;
}

void LevelManager::CreatePlayerObjectComponents(PlayerObject& playerObject, const Vector3& position) const {
	CapsuleVolume* volume = new CapsuleVolume(1.4f, 1.0f);

	playerObject.SetBoundingVolume((CollisionVolume*)volume);

	playerObject.GetTransform()
		.SetScale(Vector3(PLAYER_MESH_SIZE, PLAYER_MESH_SIZE, PLAYER_MESH_SIZE))
		.SetPosition(position);

	playerObject.SetRenderObject(new RenderObject(&playerObject.GetTransform(), mEnemyMesh, mKeeperAlbedo, mKeeperNormal, mBasicShader, PLAYER_MESH_SIZE));
	playerObject.SetPhysicsObject(new PhysicsObject(&playerObject.GetTransform(), playerObject.GetBoundingVolume(), 1, 1, 5));


	playerObject.GetPhysicsObject()->SetInverseMass(PLAYER_INVERSE_MASS);
	playerObject.GetPhysicsObject()->InitSphereInertia(false);

	playerObject.SetCollisionLayer(Player);
}

GuardObject* LevelManager::AddGuardToWorld(const Vector3 position, const std::string& guardName) {
	GuardObject* guard = new GuardObject(guardName);

	float meshSize = PLAYER_MESH_SIZE;
	float inverseMass = PLAYER_INVERSE_MASS;

	CapsuleVolume* volume = new CapsuleVolume(1.3f, 1.0f);
	guard->SetBoundingVolume((CollisionVolume*)volume);

	guard->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	guard->SetRenderObject(new RenderObject(&guard->GetTransform(), mEnemyMesh, mKeeperAlbedo, mKeeperNormal, mBasicShader, meshSize));
	guard->SetPhysicsObject(new PhysicsObject(&guard->GetTransform(), guard->GetBoundingVolume(), 1, 0, 5));

	guard->GetPhysicsObject()->SetInverseMass(PLAYER_INVERSE_MASS);
	guard->GetPhysicsObject()->InitSphereInertia(false);

	guard->SetCollisionLayer(Npc);

	guard->SetPlayer(mTempPlayer);
	guard->SetGameWorld(mWorld);

	mWorld->AddGameObject(guard);
	mUpdatableObjects.push_back(guard);

	return guard;
}