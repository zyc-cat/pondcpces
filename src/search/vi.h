/* vi.h */

StateList *CreateAllStateList();
StateList *CreateAllStateListWithoutGoal();
void ValueIteration (StateList *list, int MaxIter);
