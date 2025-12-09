#pragma once
#include <map>
#include <string>

class Animator
{
public:
    Animator(int hmodel);
    ~Animator();

    void Update();

    void AddFile(int id, std::string filename, bool loop, float speed = 1.0f, int animNo = 0);
    void Play(int id, float margeTime = 0.2f);

    void SetPlaySpeed(float speed);
    float GetPlaySpeed();

    bool IsFinish();

    int GetCurrentID();
    float GetCurrentFrame();
    float GetMaxFrame();

    int GetCurrentAnim() const { return currentAnim; }

    // 旧コード互換用
    float GetTotalFrame() { return GetMaxFrame(); }

private:
    int baseModel;

    struct FileInfo {
        int hModel;
        bool loop;
        float maxFrame;
        float playSpeed;
        int animNo;
        FileInfo() : hModel(-1), loop(false), maxFrame(1.0f), playSpeed(1.0f), animNo(0) {}
    };
    std::map<int, FileInfo> fileInfos;

    struct PlayInfo {
        int fileID;
        int attachID;
        float frame;
        PlayInfo() : fileID(-1), attachID(-1), frame(0.0f) {}
    };
    PlayInfo current;
    PlayInfo before;

    float blendTime;
    float blendTimeMax;

    bool finished;
    float playSpeed;
    int hModel;

    std::map<int, int> animHandles;

    int currentAnim;
};
