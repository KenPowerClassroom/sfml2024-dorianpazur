#include "pch.h"

const int HEIGHT = 25;
const int WIDTH = 40;
const int tileSize = 18;

#include"../16_SFML_Games/outrun.h"
#include"../16_SFML_Games/outrun_camera.h"

TEST(Camera, SpeedsUpWhenHoldingAccelerator) {
	Camera camera;

	float prevSpeed = camera.speed();

	for (int i = 0; i < 60; i++) // presimulate for 60 frames
	{
		camera.resetModifiers();
		camera.holdAccelerator(); // simulate input
		camera.update();
		EXPECT_NE(camera.speed(), 0.0f); // expect that it moved
		EXPECT_GE(camera.speed(), prevSpeed); // expect that the speed is increasing or equal to previous
		prevSpeed = camera.speed();
	}
}

TEST(Camera, SlowsDownWhenNotAccelerating) {
	Camera camera;

	for (int i = 0; i < 60; i++) // presimulate for 60 frames
	{
		camera.resetModifiers();
		camera.holdAccelerator(); // simulate input
		camera.update();
	}

	float prevSpeed = camera.speed();

	EXPECT_NE(prevSpeed, 0.0f); // should be at speed now

	for (int i = 0; i < 60; i++) // simulate for 60 frames
	{
		camera.resetModifiers();
		camera.update();
		if (camera.speed() == 0.0f)
		{
			break; // break out when speed reaches 0
		}
		EXPECT_LT(camera.speed(), prevSpeed); // expect that the speed is decreasing
		prevSpeed = camera.speed();
	}
}

TEST(Camera, StaysInBoundsHorizontally) {
	Camera camera;

	for (int i = 0; i < 240; i++) // presimulate for 240 frames
	{
		camera.resetModifiers();
		camera.strafeLeft(); // simulate input
		camera.update();
		EXPECT_LT(camera.xPos(), 0.0f); // expect that it moved
		EXPECT_GE(camera.xPos(), -X_POS_RANGE); // expect that it never exceeds the range
	}

	EXPECT_EQ(camera.xPos(), -X_POS_RANGE); // expect that it moved to the boundary

	for (int i = 0; i < 240; i++) // simulate for 240 frames
	{
		camera.resetModifiers();
		camera.strafeRight(); // simulate input
		camera.update();
		EXPECT_GT(camera.xPos(), -X_POS_RANGE); // expect that it moved
		EXPECT_LE(camera.xPos(), X_POS_RANGE); // expect that it never exceeds the range
	}

	EXPECT_EQ(camera.xPos(), X_POS_RANGE); // expect that it moved to the boundary
}

TEST(Camera, BrakesAndReverses) {
	Camera camera;

	float prevSpeed = camera.speed();

	for (int i = 0; i < 60; i++) // presimulate for 60 frames
	{
		camera.resetModifiers();
		camera.holdAccelerator(); // simulate input
		camera.update();
		EXPECT_NE(camera.speed(), 0.0f); // expect that it moved
		EXPECT_GE(camera.speed(), prevSpeed); // expect that the speed is increasing or equal to previous
		prevSpeed = camera.speed();
	}
	
	prevSpeed = camera.speed();

	for (int i = 0; i < 240; i++) // simulate for 240 frames (it takes some time to slow down)
	{
		camera.resetModifiers();
		camera.holdBrakes(); // simulate input
		camera.update();
		EXPECT_LT(camera.speed(), prevSpeed); // expect that the speed is decreasing and eventually becomes negative
		prevSpeed = camera.speed();
	}

	EXPECT_LT(camera.speed(), 0.0f); // expect that the speed is negative (reversing)
}
