#pragma once

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <limits>

struct PoseParameters
{
public:
	float XTranslation;
	float YTranslation;
	float ZTranslation;
	float XRotation;
	float YRotation;
	float ZRotation;
	
	PoseParameters() : XTranslation{0.0f}, YTranslation{0.0f}, ZTranslation{0.0f}, XRotation{0.0f}, YRotation{0.0f}, ZRotation{0.0f} {}

	PoseParameters(float xtrans, float ytrans, float ztrans, float xrot, float yrot, float zrot) : XTranslation{xtrans}, YTranslation{ytrans}, ZTranslation{ztrans}, XRotation{xrot}, YRotation{yrot}, ZRotation{zrot} {}

	PoseParameters(const PoseParameters &params)
	{
		XTranslation = params.XTranslation;
		YTranslation = params.YTranslation;
		ZTranslation = params.ZTranslation;
		XRotation = params.XRotation;
		YRotation = params.YRotation;
		ZRotation = params.ZRotation;
	}

	PoseParameters operator+(PoseParameters const &obj) const
	{
		return PoseParameters(XTranslation + obj.XTranslation, YTranslation + obj.YTranslation, ZTranslation + obj.ZTranslation, XRotation + obj.XRotation, YRotation + obj.YRotation, ZRotation + obj.ZRotation);	
	}
	
	PoseParameters operator-(PoseParameters const &obj) const
	{
		return PoseParameters(XTranslation - obj.XTranslation, YTranslation - obj.YTranslation, ZTranslation - obj.ZTranslation, XRotation - obj.XRotation, YRotation - obj.YRotation, ZRotation - obj.ZRotation);	
	}

	PoseParameters scale(float c)
	{
		return PoseParameters(c * XTranslation, c * YTranslation, c * ZTranslation, c * XRotation, c * YRotation, c * ZRotation);
	}


	// For debugging only
	void Print()
	{
		std::cout << "XTranslation: " << XTranslation << " YTranslation: " << YTranslation << " ZTranslation: " << ZTranslation << " XRotation: " << XRotation << " YRotation: " << YRotation << " ZRotation: " << ZRotation << std::endl;
	}
};

class Particle {

public:
	PoseParameters Position;
	float BestEnergyScore;
	PoseParameters BestPosition;
	PoseParameters Velocity;

	Particle(PoseParameters initialPosition, float energy): Position{initialPosition}, BestEnergyScore{energy}, BestPosition{initialPosition}, Velocity{PoseParameters()} {}

	Particle(): Position{PoseParameters()}, BestEnergyScore{std::numeric_limits<float>::infinity()}, BestPosition{PoseParameters()}, Velocity{PoseParameters()} {}

};

float** GenerateMapsFromParticles(int numParams, Particle* particles);
float** GenerateMapsFromPoseParameters(int numParams, PoseParameters* poseparams);
float CalculateEnergy(float* depthImage1, float* depthImage2, int size);

class PSO {

private:	
	Particle* particles;
	float* referenceImage;
	int populationSize;
	int iterations;
	int ImageWidth;
	int ImageHeight;
	float CognitiveConst;
	float SocialConst;
	float ConstrictionConst;
	PoseParameters GlobalBestPosition;
	float GlobalBestEnergy;

public:
	PSO(PoseParameters* parameterList, float* refImg, int numParticles, int iters, int ImgWidth, int ImgHeight, float CogConst=2.8, float SocConst=1.3) : referenceImage{refImg}, populationSize{numParticles}, iterations{iters}, ImageWidth{ImgWidth}, ImageHeight{ImgHeight}, CognitiveConst{CogConst}, SocialConst{SocConst}, ConstrictionConst{0.0f}, GlobalBestPosition{PoseParameters()}, GlobalBestEnergy{std::numeric_limits<float>::infinity()} {
		float Phi = CognitiveConst + SocialConst;
		if (Phi <= 4) 
		{
			std::cerr << "WARNING: Optimization constants too small" << std::endl;
		}
		ConstrictionConst = 2.0f / std::abs(2.0f - Phi - sqrt(Phi * Phi - 4 * Phi)); 
		
		// Intialize particles
		particles = new Particle[numParticles];
		for (int i = 0; i < numParticles; i++)
		{
			particles[i].Position = parameterList[i];
		}
	}

	PoseParameters Run()
	{
		for (int generation = 0; generation < iterations; generation++)
		{
			std::cout << "Current Global best energy: " << GlobalBestEnergy << std::endl;
			float** DepthImages = GenerateMapsFromParticles(populationSize, particles);
			for (int p = 0; p < populationSize; p++)
			{
					float currentIndividualEnergy = CalculateEnergy(referenceImage, DepthImages[p], ImageWidth*ImageHeight);
					if (currentIndividualEnergy < particles[p].BestEnergyScore)
					{
						particles[p].BestEnergyScore = currentIndividualEnergy;
						particles[p].BestPosition = particles[p].Position;
						if (currentIndividualEnergy < GlobalBestEnergy) 
						{
							GlobalBestEnergy = currentIndividualEnergy;
							GlobalBestPosition = particles[p].Position;
						}
					}	
					float r1 = ((float) std::rand() / RAND_MAX);
			    float r2 = ((float) std::rand() / RAND_MAX);
					particles[p].Velocity = particles[p].Velocity + ((particles[p].BestPosition - particles[p].Position).scale(CognitiveConst).scale(r1) + (GlobalBestPosition - particles[p].Position).scale(SocialConst).scale(r2)).scale(ConstrictionConst);
					particles[p].Position = particles[p].Position + particles[p].Velocity;
			}
		}
		return GlobalBestPosition;
	}

};


