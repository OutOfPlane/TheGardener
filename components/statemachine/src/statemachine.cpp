#include <statemachine.hpp>

using namespace gardener;

condition::condition(const char *name, bool (*testAction)(void)) : gardenObject(name)
{
    _testAction = testAction;
}

void condition::printState()
{
    printf("%s is %s\r\n", _name, *this ? "(TRUE)" : "(FALSE)");
}

statemachine::statemachine(const char *name, uint32_t numStates)
    : gardenObject(name), _numStates(numStates)
{
    _currentState = 0;
    _enterStateCallbacks = new enterStateCB[numStates];
    for (size_t i = 0; i < _numStates; i++)
    {
        _enterStateCallbacks[i] = nullptr;
    }
    _conditionList = new condition *[numStates * numStates];

    for (size_t i = 0; i < _numStates * _numStates; i++)
    {
        _conditionList[i] = nullptr;
    }
}

void statemachine::start(uint32_t initialState)
{
    if(initialState < _numStates){
        _changeState(initialState);
    }    
}

void statemachine::autoNext()
{
    for (size_t i = 0; i < _numStates; i++)
    {
        condition *tst = _conditionList[_currentState * _numStates + i];
        if (tst)
        {
            // condition not null
            // tst->printState();
            if (*tst)
            {
                if (_transitionCallback)
                {
                    if (_transitionCallback(_currentState, i))
                    {
                        _changeState(i);
                    }
                }
                else
                {
                    _changeState(i);
                }
            }
        }
    }
}

bool statemachine::changeState(uint32_t nextState)
{
    if (nextState >= _numStates)
        return false;
    if (nextState == _currentState)
        return true;

    if (_conditionList)
    {
        condition *tst = _conditionList[_currentState * _numStates + nextState];
        if (tst)
        {
            // condition not null
            // tst->printState();
            if (*tst)
            {
                if (_transitionCallback)
                {
                    if (_transitionCallback(_currentState, nextState))
                    {
                        _changeState(nextState);
                        return true;
                    }
                }
                else
                {
                    _changeState(nextState);
                    return true;
                }
            }
        }
        else
        {
            // null condition == transition not allowed
        }
    }
    return false;
}

void statemachine::onTransitionCB(bool (*transitionCallback)(uint32_t state, uint32_t nextState))
{
    _transitionCallback = transitionCallback;
}

void statemachine::onEnterStateCB(uint32_t state, void (*enterStateCallback)(void))
{
    if (state < _numStates)
    {
        _enterStateCallbacks[state] = enterStateCallback;
    }
}

void statemachine::setTransitionCondition(uint32_t fromState, uint32_t toState, condition *cond)
{
    if ((fromState < _numStates) && (toState < _numStates))
    {
        _conditionList[fromState * _numStates + toState] = cond;
    }
}

void statemachine::_changeState(uint32_t nextState)
{
    _currentState = nextState;
    if (_enterStateCallbacks[nextState])
    {
        _enterStateCallbacks[nextState]();
    }
}
