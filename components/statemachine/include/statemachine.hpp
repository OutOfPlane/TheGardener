#ifndef G_STATEMACHINE_H
#define G_STATEMACHINE_H

#include <gardenObject.hpp>

namespace gardener
{
    class condition : public gardenObject
    {
    public:
        condition(const char *name, bool (*testAction)(void));

        virtual ~condition()
        {
        }

        virtual operator bool() const
        {
            if (_testAction)
                return _testAction();
            else
                return false;
        }

        void printState();

    private:
        bool (*_testAction)(void) = nullptr;
    };

    class statemachine : public gardenObject
    {
        typedef void (*enterStateCB)(void);

    public:
        statemachine(const char *name, uint32_t numStates);
        void start(uint32_t initialState = 0);
        void autoNext();
        bool changeState(uint32_t nextState);
        void onTransitionCB(bool (*transitionCallback)(uint32_t state, uint32_t nextState));
        void onEnterStateCB(uint32_t state, void (*enterStateCallback)(void));
        void setTransitionCondition(uint32_t fromState, uint32_t toState, condition *cond);

    private:
        bool (*_transitionCallback)(uint32_t state, uint32_t nextState) = nullptr;

        enterStateCB *_enterStateCallbacks;
        uint32_t _numStates;
        uint32_t _currentState;
        condition **_conditionList;

        void _changeState(uint32_t nextState);
    };

} // namespace statemachine

#endif