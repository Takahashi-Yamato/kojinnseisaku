#pragma once
class GameObject;

class StateBase {
public:
	StateBase(GameObject* base) { baseObject = base; }
	virtual ~StateBase() {}
	virtual void Resume() {} // ÄŠJ
	virtual void Update() {} // XV
	virtual void Suspend() {} // ˆê’â~
protected:
	GameObject* baseObject;
	template<class C>
	C* GetBase() {
		return dynamic_cast<C*>(baseObject);
	}
};