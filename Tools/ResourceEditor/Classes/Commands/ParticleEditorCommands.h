/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __PARTICLE_EDITOR_COMMANDS_H__
#define __PARTICLE_EDITOR_COMMANDS_H__

#include <DAVAEngine.h>
#include "Command.h"

using namespace DAVA;

/////////////////////////////////////////////////////////////////////////////////////////////
// Yuri Coder, 03/12/2012. New commands for Particle Editor QT.

// Add new Particle Emitter.
class CommandAddParticleEmitter: public Command
{
public:
	CommandAddParticleEmitter();

protected:
    
	virtual void Execute();
};

// Start/stop/restart Particle Effect.
class CommandStartStopParticleEffect: public Command
{
public:
	CommandStartStopParticleEffect(bool isStart);
    
protected:
	virtual void Execute();
    bool isStart;
};

class CommandRestartParticleEffect: public Command
{
public:
	CommandRestartParticleEffect();
    
protected:
	virtual void Execute();
};


// Add new layer to Particle Emitter.
class CommandAddParticleEmitterLayer: public Command
{
public:
	CommandAddParticleEmitterLayer();
    
protected:
    
	virtual void Execute();
};

// Remove a layer from Particle Emitter.
class CommandRemoveParticleEmitterLayer: public Command
{
public:
	CommandRemoveParticleEmitterLayer();
    
protected:

	virtual void Execute();
};

// Clone a layer inside Particle Emitter.
class CommandCloneParticleEmitterLayer: public Command
{
public:
	CommandCloneParticleEmitterLayer();
    
protected:
    
	virtual void Execute();
};

// Add new force to Particle Emitter layer.
class CommandAddParticleEmitterForce: public Command
{
public:
	CommandAddParticleEmitterForce();
    
protected:
    
	virtual void Execute();
};

// Remove a force from Particle Emitter layer.
class CommandRemoveParticleEmitterForce: public Command
{
public:
	CommandRemoveParticleEmitterForce();
    
protected:
    
	virtual void Execute();
};

class CommandUpdateEffect: public Command
{
public:
	CommandUpdateEffect(ParticleEffectComponent* particleEffect);
	void Init(float32 playbackSpeed, bool stopOnLoad);
	
protected:
	virtual void Execute();
	
private:
	ParticleEffectComponent* particleEffect;

	float32 playbackSpeed;
	bool stopOnLoad;
};

class CommandUpdateEmitter: public Command
{
public:
	CommandUpdateEmitter(ParticleEmitter* emitter);
	void Init(ParticleEmitter::eType emitterType,
			  RefPtr<PropertyLine<float32> > emissionRange,
			  RefPtr<PropertyLine<Vector3> > emissionVector,
			  RefPtr<PropertyLine<float32> > radius,
			  RefPtr<PropertyLine<Color> > colorOverLife,
			  RefPtr<PropertyLine<Vector3> > size,
			  float32 life,
			  float32 playbackSpeed);

protected:
	virtual void Execute();
	
private:
	ParticleEmitter* emitter;

	ParticleEmitter::eType emitterType;
	RefPtr<PropertyLine<float32> > emissionRange;
	RefPtr<PropertyLine<Vector3> > emissionVector;
	RefPtr<PropertyLine<float32> > radius;
	RefPtr<PropertyLine<Color> > colorOverLife;
	RefPtr<PropertyLine<Vector3> > size;
	float32 life;
	float32 playbackSpeed;
};

class CommandUpdateParticleLayer: public Command
{
public:
	CommandUpdateParticleLayer(ParticleEmitter* emitter, ParticleLayer* layer);
	DAVA_DEPRECATED(void Init(const QString& layerName,	//DEPRECATE: using QString
			  ParticleLayer::eType layerType,
			  bool isDisabled,
			  bool additive,
  			  bool isLong,
			  Sprite* sprite,
			  RefPtr< PropertyLine<float32> > life,
			  RefPtr< PropertyLine<float32> > lifeVariation,
			  RefPtr< PropertyLine<float32> > number,
			  RefPtr< PropertyLine<float32> > numberVariation,
			  RefPtr< PropertyLine<Vector2> > size,
			  RefPtr< PropertyLine<Vector2> > sizeVariation,
			  RefPtr< PropertyLine<Vector2> > sizeOverLife,
			  RefPtr< PropertyLine<float32> > velocity,
			  RefPtr< PropertyLine<float32> > velocityVariation,
			  RefPtr< PropertyLine<float32> > velocityOverLife,
			  RefPtr< PropertyLine<float32> > spin,
			  RefPtr< PropertyLine<float32> > spinVariation,
			  RefPtr< PropertyLine<float32> > spinOverLife,

			  RefPtr< PropertyLine<Color> > colorRandom,
			  RefPtr< PropertyLine<float32> > alphaOverLife,
			  RefPtr< PropertyLine<Color> > colorOverLife,
			  RefPtr< PropertyLine<float32> > angle,
			  RefPtr< PropertyLine<float32> > angleVariation,

			  float32 startTime,
			  float32 endTime,
			  bool frameOverLifeEnabled,
			  float32 frameOverLifeFPS,

			  float32 pivotPointX,
			  float32 pivotPointY));

protected:
    virtual void Execute();
	
private:
	ParticleEmitter* emitter;
	ParticleLayer* layer;

	QString layerName;
	ParticleLayer::eType layerType;
	bool isDisabled;
	bool isLong;
	bool additive;
	Sprite* sprite;
	RefPtr< PropertyLine<float32> > life;
	RefPtr< PropertyLine<float32> > lifeVariation;
	RefPtr< PropertyLine<float32> > number;
	RefPtr< PropertyLine<float32> > numberVariation;
	RefPtr< PropertyLine<Vector2> > size;
	RefPtr< PropertyLine<Vector2> > sizeVariation;
	RefPtr< PropertyLine<Vector2> > sizeOverLife;
	RefPtr< PropertyLine<float32> > velocity;
	RefPtr< PropertyLine<float32> > velocityVariation;
	RefPtr< PropertyLine<float32> > velocityOverLife;
	RefPtr< PropertyLine<float32> > spin;
	RefPtr< PropertyLine<float32> > spinVariation;
	RefPtr< PropertyLine<float32> > spinOverLife;

	RefPtr< PropertyLine<Color> > colorRandom;
	RefPtr< PropertyLine<float32> > alphaOverLife;
	RefPtr< PropertyLine<Color> > colorOverLife;
	RefPtr< PropertyLine<float32> > frameOverLife;
	RefPtr< PropertyLine<float32> > angle;
	RefPtr< PropertyLine<float32> > angleVariation;

	float32 startTime;
	float32 endTime;
	bool frameOverLifeEnabled;
	float32 frameOverLifeFPS;

	float32 pivotPointX;
	float32 pivotPointY;
};

class CommandUpdateParticleLayerTime: public Command
{
public:
	CommandUpdateParticleLayerTime(ParticleLayer* layer);
	void Init(float32 startTime, float32 endTime);

protected:
    virtual void Execute();
	
private:
	ParticleLayer* layer;
	float32 startTime;
	float32 endTime;
};

class CommandUpdateParticleLayerEnabled: public Command
{
public:
	CommandUpdateParticleLayerEnabled(ParticleLayer* layer, bool isEnabled);

protected:
    virtual void Execute();
	
private:
	ParticleLayer* layer;
	bool isEnabled;
};

class CommandUpdateParticleForce: public Command
{
public:
	CommandUpdateParticleForce(ParticleLayer* layer, uint32 forceId);
	
	void Init(RefPtr< PropertyLine<Vector3> > force,
			  RefPtr< PropertyLine<Vector3> > forcesVariation,
			  RefPtr< PropertyLine<float32> > forcesOverLife);
	
protected:
    virtual void Execute();
	
private:
	ParticleLayer* layer;
	uint32 forceId;
	
	RefPtr< PropertyLine<Vector3> > force;
	RefPtr< PropertyLine<Vector3> > forcesVariation;
	RefPtr< PropertyLine<float32> > forcesOverLife;
};

// Load/save Particle Emitter Node.
class CommandLoadParticleEmitterFromYaml : public Command
{
public:
	DAVA_DEPRECATED(CommandLoadParticleEmitterFromYaml()); // DEPRECATED: using ParticlesEditorController(QOBJECT)

protected:
    virtual void Execute();
};

class CommandSaveParticleEmitterToYaml : public Command
{
public:
	DAVA_DEPRECATED(CommandSaveParticleEmitterToYaml(bool forceAskFilename)); // DEPRECATED: using ParticlesEditorController(QOBJECT)

protected:
    virtual void Execute();
    
    bool forceAskFilename;
};

// The same for Inner Emitters.
class CommandLoadInnerEmitterFromYaml : public Command
{
public:
	DAVA_DEPRECATED(CommandLoadInnerEmitterFromYaml()); // DEPRECATED: using ParticlesEditorController(QOBJECT)
	
protected:
    virtual void Execute(); 
};

class CommandSaveInnerEmitterToYaml : public Command
{
public:
	DAVA_DEPRECATED(CommandSaveInnerEmitterToYaml(bool forceAskFilename)); // DEPRECATED: using ParticlesEditorController(QOBJECT)
	
protected:
    virtual void Execute(); 
    
    bool forceAskFilename;
};


#endif //__PARTICLE_EDITOR_COMMANDS_H__