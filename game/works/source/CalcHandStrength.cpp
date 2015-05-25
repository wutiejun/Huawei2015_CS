#include "CalcHandStrength.h"

//return Calc hand strength
//current_player_number other total player
double CalcHandStrength(int sampling_point,int win_point,int tie_point,int current_other_player_number)
{
    // to do 预估对手性格
    double hand_strength = 1;
    bool first_flag = true;
    int win = win_point + tie_point/(current_other_player_number + 1) ;
    for (int i = 0 ; i < current_other_player_number; ++i)
    {
        if (first_flag)
        {
            hand_strength = (double)(win - i)/(sampling_point - i) ;
            first_flag = false;
        }
        else
        {
            hand_strength *= (double)(win - i)/(sampling_point - i) ;
        }
    }
    return hand_strength;
}
