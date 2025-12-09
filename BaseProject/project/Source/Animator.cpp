#include "Animator.h"
#include <string>
#include <assert.h>
#include "../Library/myDxLib.h"
#include "../Library/Time.h"
#include "Object3D.h"

Animator::Animator(int model)
{
    baseModel = model;
    fileInfos.clear();
    playSpeed = 1.0f;
    blendTimeMax = 0.0f;
    blendTime = 0.0f;
    finished = true;
    this->hModel = model; // 修正
    currentAnim = -1;
}

// 以下 Update, AddFile, Play, SetPlaySpeed, GetPlaySpeed, IsFinish, GetCurrentID, GetCurrentFrame, GetMaxFrame はそのまま

Animator::~Animator()
{
}

void Animator::Update()
{
    if (current.attachID >= 0)
    {
        const FileInfo& f = fileInfos[current.fileID];
        current.frame += Time::DeltaTime() * playSpeed * f.playSpeed * 30.0f;

        if (current.frame >= f.maxFrame)
        {
            if (f.loop)
            {
                current.frame -= f.maxFrame;
            }
            else
            {
                current.frame = f.maxFrame;
                finished = true;
            }
        }
        MV1SetAttachAnimTime(baseModel, current.attachID, current.frame);
    }

    if (before.attachID >= 0)
    {
        blendTime += Time::DeltaTime();

        if (blendTime >= blendTimeMax)
        {
            MV1SetAttachAnimBlendRate(baseModel, current.attachID, 1.0f);
            MV1SetAttachAnimBlendRate(baseModel, before.attachID, 0.0f);
            MV1DetachAnim(baseModel, before.attachID);
            before.attachID = -1;
            before.fileID = -1;
            return;
        }

        const FileInfo& f = fileInfos[before.fileID];
        before.frame += Time::DeltaTime() * playSpeed * f.playSpeed * 30.0f;

        if (before.frame >= f.maxFrame)
        {
            if (f.loop)
            {
                before.frame -= f.maxFrame;
            }
            else
            {
                before.frame = f.maxFrame;
            }
        }
        MV1SetAttachAnimTime(baseModel, before.attachID, before.frame);

        float rate = blendTime / blendTimeMax;
        MV1SetAttachAnimBlendRate(baseModel, current.attachID, rate);
        MV1SetAttachAnimBlendRate(baseModel, before.attachID, 1.0f - rate);
    }
}

void Animator::AddFile(int id, std::string filename, bool loop, float speed, int animNo)
{
    FileInfo inf;
    inf.hModel = MV1LoadModel(filename.c_str());
    //assert(inf.hModel > 0);
    inf.loop = loop;
    inf.playSpeed = speed;
    inf.animNo = animNo;

    if (inf.hModel > 0)
    {
        inf.maxFrame = MV1GetAnimTotalTime(inf.hModel, animNo);
    }
    else
    {
        inf.maxFrame = 0.0f;
    }

    if (fileInfos.count(id) > 0)
    {
        MessageBox(nullptr,
            (std::string("ID=") + std::to_string(id) + "は使われています").c_str(),
            "Animator", MB_OK);
        assert(false);
    }

    fileInfos.emplace(id, inf);
}

void Animator::Play(int id, float margeTime)
{
    if (current.fileID == id)
    {
        return;
    }

    if (before.attachID >= 0)
    {
        MV1DetachAnim(baseModel, before.attachID);
        before.attachID = -1;
        before.fileID = -1;
    }

    if (current.attachID >= 0)
    {
        before = current;
        blendTimeMax = margeTime;
        blendTime = 0.0f;
    }
    else
    {
        blendTimeMax = 0.0f;
    }

    current.fileID = id;

    if (fileInfos.count(id) > 0)
    {
        // animNoを指定してアニメをアタッチ
        current.attachID = MV1AttachAnim(baseModel, fileInfos[id].animNo, fileInfos[id].hModel);
        current.frame = 0.0f;
        finished = false;
        MV1SetAttachAnimTime(baseModel, current.attachID, 0.0f);

        if (before.attachID >= 0)
        {
            MV1SetAttachAnimBlendRate(baseModel, current.attachID, 0.0f);
            MV1SetAttachAnimBlendRate(baseModel, before.attachID, 1.0f);
        }
        else
        {
            MV1SetAttachAnimBlendRate(baseModel, current.attachID, 1.0f);
        }
    }
}

void Animator::SetPlaySpeed(float speed)
{
    playSpeed = speed;
}

float Animator::GetPlaySpeed()
{
    return playSpeed;
}

bool Animator::IsFinish()
{
    return finished;
}

int Animator::GetCurrentID()
{
    return current.fileID;
}

float Animator::GetCurrentFrame()
{
    return current.frame;
}

float Animator::GetMaxFrame()
{
    if (current.attachID >= 0)
    {
        return fileInfos[current.fileID].maxFrame;
    }
    return 0.0f;
}
