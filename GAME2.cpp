// Future_Space_Frogs.cpp: A program using the TL-Engine

#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <iostream>  // enables the use of cout and end line
#include <time.h>	    // Used for random number generating and timers
#include <sstream> // enables the use of string stream
#include <vector>	    // Dynamic Arrays - Better than standard arrays if values in the sequence need changing
using namespace tle;
using namespace std; // Standard definitions

enum portalSide { leftSide, rightSide, frontSide, backSide, noSide };
enum gameStates { playing, paused, won, over };
enum frogIdentifier { FirstFrog, SecondFrog, ThirdFrog, FourthFrog }; // Identifying frogs
enum firstFrogStates { waiting, moving, dead, flipping, escaped };
portalSide sphere2Box(float sphereXPos, float sphereZPos, float oldXPos, float oldZPos, float sphereRad, float boxXPos, float boxZPos, float boxWidth, float boxDepth);

//bool sphere2Box(float sphereXPos, float sphereZPos, float oldXPos, float oldZPos, float sphereRad, float boxXPos, float boxZPos, float boxWidth, float boxDepth);
bool sphere2Point(float frog1X, float frog1Z, float frog2X, float frog2Z, float roverX, float roverZ, float radius);
bool point2Box(float frog1X, float frog1Z, float frog2X, float frog2Z, float roverX, float roverZ, float roverRad);
bool sphere2Sphere(float sphere1XPos, float sphere1ZPos, float sphere1Rad, float sphere2XPos, float sphere2ZPos, float sphere2Rad);

// frog details NEW //

const int kFrogNum = 4;				// Number of frogs
bool frogFlipping = false;				// Set frog flipping to true or false

int frogStateTracker[kFrogNum];		// Frog game state
int currentFrog = 0;				   // Current Frog
IModel* frog[kFrogNum];				// Model for each frog

//void EnterFlippingPhase()				// Activate frog flipping
//{												
//	frogFlipping = true;
//	frog[currentFrog]->RotateLocalZ(180.0f);
//}

//void LeaveFlippingPhase()			// Leave frog flipping stage
//{	// Activate frog flipping
//	frogFlipping = false;
//	frog[currentFrog]->RotateLocalZ(0.0f);
//}

void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine(kTLX);
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder("Media");

	/**** Set up your scene here ****/

	// constants //

	const float kSpeedInc = 0.05f;
	const float kSphereRadius = 4.0f;
	const float kBoxRadius = 4.0f;
	const float kGameSpeed = 0.50f;
	const float kPortalScale = 1.2f;
	
	// camera positions
	const float kCamYPos = 120.0f;
	const float kCamZPos = -100.0f;
	const float kCamRot = 50.0f;

	// frogs Initial Rotation
	const float kFrog1IntialRot = 90.0f;
	const float kFrog2IntialRot = -90.0f;
	const float kFrog3IntialRot = 180.0f;

	// Frogs final rotation
	const float kFrog1FinalRot = 180.0f;
	const float kFrog2FinalRot = 180.0f;
	const float kFrog3FinalRot = 180.0f;
	const float kFrog4FinalRot = 180.0f;

	// Rover positions
	const float kRoverZPos = -50.0f;
	const float kIslandYPos = -6.0f;
	const float kWaterYPos = -5.0f;
	const float kSkyYPos = -960.0f;

	// EKeyCode's for movement of the car

	const EKeyCode kMoveForwardsKey = Key_W;
	const EKeyCode kMoveBackwardsKey = Key_S;
	const EKeyCode kMoveLeftKey = Key_A;
	const EKeyCode kMoveRightKey = Key_D;
	const EKeyCode kPauseKey = Key_P;
	const EKeyCode kQuitKey = Key_Escape;

	// Island Max Boundaries

	const float kIslandMaxXPos = 60.0f;
	const float kIslandMaxZPos = 60.0f;

	// Island Min Boundaries

	const float kIslandMinXPos = -60.0f;
	const float kIslandMinZPos = -60.0f;

	// floats //

	float roverRot = 3.5;
	float roverRadius = 1.0f;
	float frogWidth = 1.0f;
	float frogDepth = 1.0f;
	float portalWidth = 2.0f;
	float portalDepth = 2.0f;
	float frogAwarenessRadius = 4.0f;
	float frogRadius = 1.5f;
	const int kFrogArraySize = 1;
	float gameSpeed = 0.50;
	float frog1Speed = 0.25;
	float frog2Speed = 0.25;
	float frog3Speed = 0.25;
	float frog4Speed = 0.25;

	// sky model
	IMesh* skyMesh = myEngine->LoadMesh("skybox.x");
	IModel* sky = skyMesh->CreateModel(0, kSkyYPos, 0);

	// floor of water model
	IMesh* floorMesh = myEngine->LoadMesh("surface.x");
	IModel* floor = floorMesh->CreateModel(0, kWaterYPos, 0);

	// island model
	IMesh* islandMesh = myEngine->LoadMesh("island.x");
	IModel* island = islandMesh->CreateModel(0, kIslandYPos, 0);

	// frog 1 model
	IMesh* frogMesh = myEngine->LoadMesh("frog.x");
	
	IModel* frog[kFrogNum];
	float frogXs[kFrogNum] = { 5, -5, 0, 0 };
	float frogYs[kFrogNum] = { 0, 0, 0, 0 };
	float frogZs[kFrogNum] = { 0, 0, -5, 5 };
	
	for (int i = 0; i < kFrogNum; i++)
	{
		frog[i] = frogMesh->CreateModel(frogXs[i], frogYs[i], frogZs[i]);
	}
	frog[0]->RotateLocalY(kFrog1IntialRot);
	frog[1]->RotateLocalY(kFrog2IntialRot);
	frog[2]->RotateLocalY(kFrog3IntialRot);

	// rover model
	IMesh* roverMesh = myEngine->LoadMesh("rover.x");
	IModel* rover = roverMesh->CreateModel(0, 0, kRoverZPos);

	// portal
	IMesh* portalMesh = myEngine->LoadMesh("portal.x");
	//IModel* portal = portalMesh->CreateModel(30.0f, 0.0f, 30.0f);

	// portal array //
	const int kNumPortal = 1;
	float portalXs[kNumPortal] = { 30.0 };
	float portalZs[kNumPortal] = { 30.0 };

	IModel* portal[kNumPortal];
	for (int i = 0; i < kNumPortal; i++)
	{
		portal[i] = portalMesh->CreateModel(portalXs[i], 0.0f, portalZs[i]);
		portal[i]->Scale(kPortalScale);
	}

	float kFrogSpeed = 0.25;

	// gamestate
	gameStates currentState = playing;
	firstFrogStates frogStateTrackers = moving;


	// create camera and rotate it around x axis
	ICamera* myCamera = myEngine->CreateCamera(kManual, 0.0f, kCamYPos, kCamZPos);  // define constant floats for final submission
	myCamera->RotateLocalX(kCamRot);

	//myCamera->AttachToParent(rover);

	//Allow game to check if keys are pressed/held.
	bool isPaused = false; //P key, pause game.
	float speedController = 1.0f;   // define constant float for final submission

	// display text
	stringstream outText;
	IFont* myFont = myEngine->LoadFont("Comic Sans MS", 24);
	int score = 0;
	int addOrNot = true;
	bool Rotation = true;

	bool canRotate[kFrogNum];		// Stores state to which frogs can rotate

	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();

		float deltaTime = myEngine->Timer(); // returns difference in time

		// display game text
		switch (currentState)
		{
		case playing:
			outText << "Score: " << score;
			myFont->Draw(outText.str(), 20, 20);
			outText.str("");
			break;
		case paused:
			outText << "Paused: " << score;
			myFont->Draw(outText.str(), 20, 20);
			outText.str("");
			break;
		case won:
			outText << "Game Won: " << score;
			myFont->Draw(outText.str(), 20, 20);
			outText.str("");
			break;
		case over:
			outText << "Game Over: " << score;
			myFont->Draw(outText.str(), 20, 20);
			outText.str("");
			break;
		}

		/**** Update your scene each frame here ****/

		// save old position of car

		float oldX = rover->GetX();
		float oldZ = rover->GetZ();

		// save old position of frog1
		float oldFrog1X = frog[0]->GetX();
		float oldFrog1Z = frog[0]->GetZ();
		float oldFrog2X = frog[1]->GetX();
		float oldFrog2Z = frog[1]->GetZ();

		// Move models

		//check if player presses pause key, invert pause variable to make game pause.
		if (isPaused == false) {
			if (myEngine->KeyHit(kMoveForwardsKey)) {
				speedController += kSpeedInc;
			}
			else if (myEngine->KeyHit(kMoveBackwardsKey)) {
				speedController -= kSpeedInc;
			}

			// Move car on keypresses
			if (myEngine->KeyHit(kMoveRightKey))
			{
				rover->MoveLocalX(gameSpeed * kGameSpeed);
				rover->RotateLocalY(roverRot);  // assign a name, smaller rotation value // 
			}
			if (myEngine->KeyHit(kMoveLeftKey))
			{
				rover->MoveLocalX(-gameSpeed * kGameSpeed);
				rover->RotateLocalY(-roverRot);
			}
			if (myEngine->KeyHeld(kMoveForwardsKey))
			{
				rover->MoveLocalZ(gameSpeed * kGameSpeed);
			}
			if (myEngine->KeyHeld(kMoveBackwardsKey))
			{
				rover->MoveLocalZ(-gameSpeed * kGameSpeed);
			}

			// check the boundaries and if the car or the frogs go out of //
			// bounds, change the game state to game over //

			if (rover->GetLocalX() >= kIslandMaxXPos || rover->GetLocalZ() >= kIslandMaxZPos || rover->GetLocalX() <= kIslandMinXPos || rover->GetLocalZ() <= kIslandMinZPos)
			{
				currentState = over;
			}
			else if (frog[0]->GetLocalX() >= kIslandMaxXPos || frog[0]->GetLocalX() <= kIslandMinXPos && frog[1]->GetLocalX() >= kIslandMaxXPos || frog[1]->GetLocalX() <= kIslandMinXPos)
			{
				currentState = over;
			}

			// check collisions
			for (int i = 0; i < kNumPortal; i++)
			{
				portalSide collision = sphere2Box(rover->GetX(), rover->GetZ(), oldX, oldZ, roverRadius,
					portalXs[i], portalZs[i], portalWidth, portalDepth);

				// resolve collisions
				if (collision == frontSide || collision == backSide)
				{
					rover->SetZ(oldZ);
				}
				else if (collision == leftSide || collision == rightSide)
				{
					rover->SetX(oldX);
				}

				// behaviour for playing state
				switch (currentState)
				{
				case playing:
				{
					frog[0]->MoveLocalZ(frog1Speed * deltaTime);
					frog[1]->MoveLocalZ(frog2Speed * deltaTime);
					frog[2]->MoveLocalZ(frog3Speed * deltaTime);
					frog[3]->MoveLocalZ(frog4Speed * deltaTime);
					frogStateTracker[0] = moving;
					frogStateTracker[1] = moving;
					//currentFrog2State = moving2;
					// check for events causing state change
					// proximity to frogs
					if (sphere2Sphere(rover->GetX(), rover->GetZ(), roverRadius, frog[0]->GetX(), frog[0]->GetZ(), frogAwarenessRadius))
					{
						// collision occured
						frog[0]->MoveLocalZ(frog1Speed * deltaTime);
						frog[1]->MoveLocalZ(frog2Speed * deltaTime);
						//frogFlipping = true;
						//currentState = paused;

						// touching frog
						if (sphere2Sphere(rover->GetX(), rover->GetZ(), roverRadius, frog[0]->GetX(), frog[0]->GetZ(), frogRadius && frogStateTracker[0] == moving && frogStateTrackers == moving))
						{
							frogStateTracker[0] = dead;
							frogStateTracker[0] = flipping;
							//canRotate[i] = true;
							//frog[i]->RotateLocalZ(180);   // define constant float for final submission
							//frogFlipping = true;
							/* frog1Speed = 0;
							score += 10;*/
							if (sphere2Sphere(rover->GetX(), rover->GetZ(), roverRadius, frog[0]->GetX(), frog[0]->GetZ(), frogRadius && FirstFrog == dead && addOrNot == false && FirstFrog == flipping && frogFlipping == true))
							{
								addOrNot = true;
								score = +10;
								frogStateTracker[0] = dead;
								canRotate[i] = true;
								frog[0]->RotateLocalZ(kFrog1FinalRot);
								//canRotate[0] = false;
								frog[0]->RotateLocalZ(0);
								//LeaveFlippingPhase();
								//frogFlipping = false;


								frog1Speed = 0;
								//frog[FirstFrog]->RotateY(3.0f);

								//frogFlipping = true;

								//Rotation = false;
								//frog[FirstFrog]->RotateLocalZ(0.0f);  // check if frog is alivee before adding score

							}
							canRotate[0] = false;
							frog[0]->RotateLocalZ(0);
						}
						// touching frog

						else if (sphere2Sphere(rover->GetX(), rover->GetZ(), roverRadius, frog[1]->GetX(), frog[1]->GetZ(), frogRadius && frogStateTracker[1] == moving))
						{
							//EnterFlippingPhase();
							//frog[SecondFrog]->RotateLocalZ(180.0f);
							score += 10;
							frogStateTracker[1] = dead;
							//frog2Speed = 0;
							if (sphere2Sphere(rover->GetX(), rover->GetZ(), roverRadius, frog[1]->GetX(), frog[1]->GetZ(), frogRadius && frogStateTracker[1] == dead && addOrNot == false))
							{
								addOrNot = true;
								score = + 10;
								frogStateTracker[1] = dead;
								//LeaveFlippingPhase();
								frog2Speed = 0;
								//Rotation = false;
								//frog[FirstFrog]->RotateLocalZ(180.0f);  // check if frog is alivee before adding score

							}
							else if (score >= 20)
							{
								currentState = won;
							}
							else if (score < 20)
							{
								currentState = over;
							}
						}
					}
					break;
				}
				case paused:
				{
					if (myEngine->KeyHit(kPauseKey)) // Change to pauseKey for final submission
					{
						outText << "Paused: " << score;
						myFont->Draw(outText.str(), 20, 20);
						outText.str("");
					}
					break;
				}
				case won:
				{
					if (frogStateTracker[0] == dead && frogStateTracker[1] == dead)
					{
						outText << "Game Won: " << score;
						myFont->Draw(outText.str(), 20, 20);
						outText.str("");
					}
					break;
				}
				case over:
				{
					if (frogStateTracker[0] != dead && frogStateTracker[1] != dead)
					{
						outText << "Game Over: " << score;
						myFont->Draw(outText.str(), 20, 20);
						outText.str("");
					}
					break;
				}
				} // end of switch (currentState)
			}

			//checking game states 
			if (currentState == over) {
				gameSpeed = 0;
				frog1Speed = 0;
				frog2Speed = 0;
				roverRot = 0;
			}
			else if (currentState == won) {
				gameSpeed = 0;
				frog1Speed = 0;
				frog2Speed = 0;
				roverRot = 0;
			}
		} // end of if (isPaused == false)

		// check if P key is hit then pause/ unpause the game
		if (myEngine->KeyHit(kPauseKey)) {     // Change to pauseKey for final submission
			if (isPaused == true) {
				isPaused = false;
			}
			else {
				isPaused = true;
			}
		}

		// exit game when escape key hit
		if (myEngine->KeyHit(kQuitKey)) {
			myEngine->Stop();
		}
	} // end of Game Loop

	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}

//void EnterFlippingPhase()
//{	// Activate frog flipping
//	frogFlipping = true;
//	frog[currentFrog]->RotateLocalZ(180.0f);
//}

bool sphere2Sphere(float sphere1XPos, float sphere1ZPos, float sphere1Rad, float sphere2XPos, float sphere2ZPos, float sphere2Rad)
{
	// variable for storing vector components
	float distX = sphere2XPos - sphere1XPos;
	float distZ = sphere2ZPos - sphere1ZPos;
	float distance = sqrt(distX * distZ + distZ * distZ);

	return (distance < (sphere1Rad + sphere2Rad));
}

portalSide sphere2Box(float sphereXPos, float sphereZPos, float oldXPos, float oldZPos, float sphereRad, float boxXPos, float boxZPos, float boxWidth, float boxDepth)
{
	float minX = boxXPos - boxWidth / 2 - sphereRad;
	float maxX = boxXPos + boxWidth / 2 + sphereRad;
	float minZ = boxZPos - boxDepth / 2 - sphereRad;
	float maxZ = boxZPos + boxDepth / 2 + sphereRad;
	portalSide result = noSide;
	if (sphereXPos > minX && sphereXPos < maxX && sphereZPos > minZ && sphereZPos < maxZ)
	{
		if (oldXPos < minX) result = leftSide;
		else if (oldXPos > maxX) result = rightSide;
		else if (oldZPos < minZ) result = frontSide;
		else					  result = backSide;
	}
	return(result);
}