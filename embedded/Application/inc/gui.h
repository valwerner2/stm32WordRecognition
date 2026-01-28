//
// Created by valwe on 1/12/26.
//

#ifndef GUI_H
#define GUI_H

#define GUI_STR_LEN 16

void Gui_Init();
void Gui_Update();


typedef struct {
    char noise_floor[GUI_STR_LEN];
    char distance[GUI_STR_LEN];
    char word[GUI_STR_LEN];
    char refresh;
}gui_info_t;


#endif //GUI_H
