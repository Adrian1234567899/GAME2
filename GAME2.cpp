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

const int frogAmount = 1;				// Number of frogs
bool frogFlipping = false;				// Set frog flipping to true or false

int frogStateTracker[frogAmount];		// Frog game state
int currentFrog = 0;				   // Current Frog
IModel* frog[frogAmount];				// Model for each frog

void EnterFlippingPhase()				// Activate frog flipping
{												
	frogFlipping = true;
	frog[currentFrog]->RotateLocalZ(180.0f);
}

void LeaveFlippingPhase()			// Leave frog flipping stage
{	// Activate frog flipping
	frogFlipping = false;
	frog[currentFrog]->RotateLocalZ(0.0f);
}

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

	// EKeyCode's for movement of the car

	const EKeyCode kMoveForwardsKey = Key_W;
	const EKeyCode kMoveBackwardsKey = Key_S;
	const EKeyCode kMoveLeftKey = Key_A;
	const EKeyCode kMoveRightKey = Key_D;

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
	IModel* sky = skyMesh->CreateModel(0, -960, 0);

	// floor of water model
	IMesh* floorMesh = myEngine->LoadMesh("surface.x");
	IModel* floor = floorMesh->CreateModel(0, -5, 0);

	// island model
	IMesh* islandMesh = myEngine->LoadMesh("island.x");
	IModel* island = islandMesh->CreateModel(0, -6, 0);

	// frog 1 model
	IMesh* frogMesh = myEngine->LoadMesh("frog.x");
	for (int i = 0; i < frogAmount; i++)
	{
		frog[FirstFrog] = frogMesh->CreateModel(5, 0, 0); // Starting point
		frog[FirstFrog]->RotateLocalY(90.0f);
		frog[SecondFrog] = frogMesh->CreateModel(-5, 0, 0); // Starting point
		frog[SecondFrog]->RotateLocalY(-90.0f);
		frog[ThirdFrog] = frogMesh->CreateModel(0, 0, -5); // Starting point
		frog[ThirdFrog]->RotateLocalY(180.0f);
		frog[FourthFrog] = frogMesh->CreateModel(0, 0, 5); // Starting point
	}

	// rover model
	IMesh* roverMesh = myEngine->LoadMesh("rover.x");
	IModel* rover = roverMesh->CreateModel(0, 0, -50);

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
		portal[i]->Scale(1.2);
	}

	float kFrogSpeed = 0.05;

	// gamestate
	gameStates currentState = playing;
	firstFrogStates frogStateTrackers = moving;


	// create camera and rotate it around x axis
	ICamera* myCamera = myEngine->CreateCamera(kManual, 0.0f, 120.0f, -100.0f);  // define constant floats for final submission
	myCamera->RotateLocalX(50.0f);

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
			else if (frog[FirstFrog]->GetLocalX() >= kIslandMaxXPos || frog[FirstFrog]->GetLocalX() <= kIslandMinXPos && frog[SecondFrog]->GetLocalX() >= kIslandMaxXPos || frog[SecondFrog]->GetLocalX() <= kIslandMinXPos)
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
					frog[FirstFrog]->MoveLocalZ(frog1Speed * deltaTime);
					frog[SecondFrog]->MoveLocalZ(frog2Speed * deltaTime);
					frog[ThirdFrog]->MoveLocalZ(frog3Speed * deltaTime);
					frog[FourthFrog]->MoveLocalZ(frog4Speed * deltaTime);
					frogStateTracker[FirstFrog] = moving;
					frogStateTracker[SecondFrog] = moving;
					//currentFrog2State = moving2;
					// check for events causing state change
					// proximity to frogs
					if (sphere2Sphere(rover->GetX(), rover->GetZ(), roverRadius, frog[FirstFrog]->GetX(), frog[FirstFrog]->GetZ(), frogAwarenessRadius))
					{
						// collision occured
						frog[FirstFrog]->MoveLocalZ(frog1Speed * deltaTime);
						frog[SecondFrog]->MoveLocalZ(frog2Speed * deltaTime);
						//frogFlipping = true;
						//currentState = paused;

						// touching frog
						if (sphere2Sphere(rover->GetX(), rover->GetZ(), roverRadius, frog[FirstFrog]->GetX(), frog[FirstFrog]->GetZ(), frogRadius && frogStateTracker[FirstFrog] == moving && frogStateTrackers == moving))
						{
							frogStateTracker[FirstFrog] = dead;
							frogStateTracker[FirstFrog] = flipping;
							//frog[FirstFrog]->RotateLocalZ(180);   // define constant float for final submission
							//frogFlipping = true;
							/* frog1Speed = 0;
							score += 10;*/
							if (sphere2Sphere(rover->GetX(), rover->GetZ(), roverRadius, frog[FirstFrog]->GetX(), frog[FirstFrog]->GetZ(), frogRadius && FirstFrog == dead && addOrNot == false && FirstFrog == flipping && frogFlipping == true))
							{
								addOrNot = true;
								score = +10;
								frogStateTracker[FirstFrog] = dead;
								frog[FirstFrog]->RotateLocalZ(180);
								LeaveFlippingPhase();
								//frogFlipping = false;


								frog1Speed = 0;
								//frog[FirstFrog]->RotateY(3.0f);

								//frogFlipping = true;

								//Rotation = false;
								//frog[FirstFrog]->RotateLocalZ(0.0f);  // check if frog is alivee before adding score

							}
						}
						// touching frog

						else if (sphere2Sphere(rover->GetX(), rover->GetZ(), roverRadius, frog[SecondFrog]->GetX(), frog[SecondFrog]->GetZ(), frogRadius && frogStateTracker[SecondFrog] == moving))
						{
							EnterFlippingPhase();
							//frog[SecondFrog]->RotateLocalZ(180.0f);
							score += 10;
							frogStateTracker[SecondFrog] = dead;
							//frog2Speed = 0;
							if (sphere2Sphere(rover->GetX(), rover->GetZ(), roverRadius, frog[SecondFrog]->GetX(), frog[SecondFrog]->GetZ(), frogRadius && frogStateTracker[SecondFrog] == dead && addOrNot == false))
							{
								addOrNot = true;
								score = + 10;
								frogStateTracker[SecondFrog] = dead;
								LeaveFlippingPhase();
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
					if (myEngine->KeyHit(Key_P)) // Change to pauseKey for final submission
					{
						outText << "Paused: " << score;
						myFont->Draw(outText.str(), 20, 20);
						outText.str("");
					}
					break;
				}
				case won:
				{
					if (frogStateTracker[FirstFrog] == dead && frogStateTracker[SecondFrog] == dead)
					{
						outText << "Game Won: " << score;
						myFont->Draw(outText.str(), 20, 20);
						outText.str("");
					}
					break;
				}
				case over:
				{
					if (frogStateTracker[FirstFrog] != dead && frogStateTracker[SecondFrog] != dead)
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
		if (myEngine->KeyHit(Key_P)) {     // Change to pauseKey for final submission
			if (isPaused == true) {
				isPaused = false;
			}
			else {
				isPaused = true;
			}
		}

		// exit game when escape key hit
		if (myEngine->KeyHit(Key_Escape)) {
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