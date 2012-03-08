/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
 *   Copyright (C) 2011  Bohdan Stelmakh <chamel@users.sourceforge.net> *
 *   Copyright (C) 2011  Mark <mentor66@users.sourceforge.net>          *
 *                                                                      *
 *    This program is free software;  you can redistribute it and / or  *
 *  modify it  under the  terms of the  GNU General  Public License as  *
 *  published by the Free Software Foundation; either version 2 of the  *
 *  License, or (at your option) any later version.                     *
 *                                                                      *
 *    This program is  distributed in the hope that it will be useful,  *
 *  but WITHOUT  ANY WARRANTY;  without even  the implied  warranty of  *
 *  MERCHANTABILITY  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 *  General Public License for more details.                            *
 *                                                                      *
 *    You can view the GNU  General Public License, online, at the GNU  *
 *  project's  web  site;  see <http://www.gnu.org/licenses/gpl.html>.  *
 *  The full text of the license is also included in the file COPYING.  *
 *                                                                      *
 ************************************************************************/

#include <stdio.h>
#include <assert.h>
#include "app.h"
#include "mod.h"
#include "selectmenu.h"

SelectMenu::SelectMenu(MenuManager * m):Menu(m, MENU_SELECT, MENU_BRIEF, "mselect.dat",
    "mselout.dat"),
cur_agent_(0), tick_count_(0), rnd_(0), sel_all_(false)
{
    tab_ = TAB_EQUIPS;
    pSelectedWeap_ = NULL;
    selectedWInstId_ = 0;
    pSelectedMod_ = NULL;
    addStatic(85, 35, 545, "#SELECT_TITLE", FontManager::SIZE_4, false);
    txtTimeId_ = addStatic(500, 9, "", FontManager::SIZE_2, true);       // Time
	moneyTxtId_ = addStatic(500, 87, 128, "0", FontManager::SIZE_2, true);     // Money

    addOption(16, 234, 129, 25, "#SELECT_RES_BUT", FontManager::SIZE_2, MENU_RESEARCH);
    teamButId_ = addToggleAction(16, 262, 129, 25, "#SELECT_TEAM_BUT", FontManager::SIZE_2, false);
    modsButId_ = addToggleAction(16, 290, 129, 25, "#MENU_MODS_BUT", FontManager::SIZE_2, false);
    equipButId_ = addToggleAction(16, 318, 129, 25, "#MENU_EQUIP_BUT", FontManager::SIZE_2, true);
	addOption(16, 346, 129, 25, "#MENU_ACC_BUT", FontManager::SIZE_2, MENU_LOADING);
    addOption(500, 347,  128, 25, "#MENU_MAIN_BUT", FontManager::SIZE_2, MENU_MAIN);

    // Team list
    pTeamLBox_ = addTeamListBox(502, 106, 124, 236, false);
    pTeamLBox_->setModel(g_App.agents().getAgents());
    // Available weapons list
    pWeaponsLBox_ = addListBox(504, 110,  122, 230, tab_ == TAB_EQUIPS);
    pWeaponsLBox_->setModel(g_App.weapons().getAvailableWeapons());
    // Available mods list
    pModsLBox_ = addListBox(504, 110,  122, 230, tab_ == TAB_MODS);
    pModsLBox_->setModel(g_App.mods().getAvalaibleMods());

    cancelButId_ = addOption(500, 270,  127, 22, "#MENU_CANCEL_BUT",
        FontManager::SIZE_2, MENU_NO_MENU, false);
    reloadButId_ = addOption(500, 295,  127, 22, "#SELECT_RELOAD_BUT",
        FontManager::SIZE_2, MENU_NO_MENU, false);
    purchaseButId_ = addOption(500, 320,  127, 22, "#SELECT_BUY_BUT",
        FontManager::SIZE_2, MENU_NO_MENU, false);
    sellButId_ = addOption(500, 320,  127, 22, "#SELECT_SELL_BUT",
        FontManager::SIZE_2, MENU_NO_MENU, false);

    // Agent name selected
    txtAgentId_ = addStatic(158, 86, "", FontManager::SIZE_2, true);
}

SelectMenu::~SelectMenu()
{
    pTeamLBox_ = NULL;
}

/*!
 * Draws a dashed line around the currently selected agent selector.
 * \param x Coordinates of the top left corner 
 * \param y Coordinates of the top left corner 
 */
void SelectMenu::drawAgentSelector(int x, int y) {
    // First create a transparent sprite
    uint8 cdata[30 * 33];
    int cwidth = 30;
    int cheight = 33;
    memset(cdata, 255, sizeof(cdata));

    // Draws the upper and lower lines
    for (int i = 0; i < cwidth; i++) {
        cdata[i] = ((rnd_ + i) % 8 <= 4) ? 252 : 16;
        cdata[i + (cheight - 1) * cwidth] =
            ((rnd_ + i) % 8 >= 4) ? 252 : 16;
    }

    // Draws the right and left line
    for (int j = 0; j < cheight; j++) {
        cdata[j * cwidth] = ((rnd_ + j) % 8 >= 4) ? 252 : 16;
        cdata[j * cwidth + cwidth - 1] = ((rnd_ + j) % 8 <= 4) ? 252 : 16;
    }

    // blits the sprite at given position
    g_Screen.scale2x(x, y, cwidth, cheight, cdata);
}

void SelectMenu::drawAgent()
{
    Agent *selected = g_Session.teamMember(cur_agent_);
    if (selected == NULL)
        return;

    int torso, arms, legs;
    int armsx = 188;
    int armsy = 152;
    int torsoy = 116;
    int legsy = 218;
    if (selected->isMale()) {
        torso = 30;
        arms = 40;
        legs = 32;
    } else {
        torso = 31;
        arms = 44;
        legs = 36;
        armsx += 10;
        armsy += 6;
        torsoy += 2;
        legsy -= 4;
    }

    if (selected->slot(Mod::MOD_LEGS)) {
        legs = selected->slot(Mod::MOD_LEGS)->icon(selected->isMale());
        getMenuFont(FontManager::SIZE_1)->drawText(366, 250,
            selected->slot(Mod::MOD_LEGS)->getName(),
            false);
    }
    if (selected->slot(Mod::MOD_ARMS)) {
        arms = selected->slot(Mod::MOD_ARMS)->icon(selected->isMale());
       getMenuFont(FontManager::SIZE_1)->drawText(366, 226,
            selected->slot(Mod::MOD_ARMS)->getName(),
            false);
    }

    menuSprites().drawSpriteXYZ(arms, armsx, armsy, 0, false, true);
    menuSprites().drawSpriteXYZ(torso, 224, torsoy, 0, false, true);
    menuSprites().drawSpriteXYZ(legs, 224, legsy, 0, false, true);

    if (selected->slot(Mod::MOD_CHEST)) {
        int chest = selected->slot(Mod::MOD_CHEST)->icon(selected->isMale());
        getMenuFont(FontManager::SIZE_1)->drawText(366, 202,
            selected->slot(Mod::MOD_CHEST)->getName(), false);
        int chestx = 216;
        int chesty = 146;
        if (!selected->isMale()) {
            chestx += 8;
            chesty += 2;
        }
        menu_manager_->menuSprites().drawSpriteXYZ(chest, chestx, chesty, 0, false,
                                          true);
    }

    if (selected->slot(Mod::MOD_HEART)) {
        int heart = selected->slot(Mod::MOD_HEART)->icon(selected->isMale());
        getMenuFont(FontManager::SIZE_1)->drawText(366, 160,
            selected->slot(Mod::MOD_HEART)->getName(), false);
        menu_manager_->menuSprites().drawSpriteXYZ(heart, 254, 166, 0, false, true);
    }

    if (selected->slot(Mod::MOD_EYES)) {
        int eyes = selected->slot(Mod::MOD_EYES)->icon(selected->isMale());
        getMenuFont(FontManager::SIZE_1)->drawText(366, 136,
            selected->slot(Mod::MOD_EYES)->getName(), false);
        int eyesx = 238;
        if (!selected->isMale()) {
            eyesx += 2;
        }
        menuSprites().drawSpriteXYZ(eyes, eyesx, 116, 0, false,
                                          true);
    }

    if (selected->slot(Mod::MOD_BRAIN)) {
        int brain = selected->slot(Mod::MOD_BRAIN)->icon(selected->isMale());
        getMenuFont(FontManager::SIZE_1)->drawText(366, 112,
            selected->slot(Mod::MOD_BRAIN)->getName(), false);
        int brainx = 238;
        if (!selected->isMale()) {
            brainx += 2;
        }
        menuSprites().drawSpriteXYZ(brain, brainx, 114, 0, false, true);
    }
    // restore lines over agent
/*    g_Screen.blit(254, 124, 30, 2,
                  background_ + 256 + 124 * GAME_SCREEN_WIDTH, false,
                  GAME_SCREEN_WIDTH);
    g_Screen.blit(264, 132, 30, 2,
                  background_ + 266 + 132 * GAME_SCREEN_WIDTH, false,
                  GAME_SCREEN_WIDTH);
    g_Screen.blit(266, 174, 36, 2,
                  background_ + 268 + 174 * GAME_SCREEN_WIDTH, false,
                  GAME_SCREEN_WIDTH);
    g_Screen.blit(252, 210, 56, 2,
                  background_ + 254 + 210 * GAME_SCREEN_WIDTH, false,
                  GAME_SCREEN_WIDTH);
    g_Screen.blit(302, 232, 10, 2,
                  background_ + 304 + 232 * GAME_SCREEN_WIDTH, false,
                  GAME_SCREEN_WIDTH);
    g_Screen.blit(264, 256, 30, 2,
                  background_ + 266 + 256 * GAME_SCREEN_WIDTH, false,
                  GAME_SCREEN_WIDTH);*/

    // write inventory
    for (int j = 0; j < 2; j++)
        for (int i = 0; i < 4; i++)
            if (j * 4 + i < selected->numWeapons()) {
                WeaponInstance *wi = selected->weapon(j * 4 + i);
                Weapon *pW = wi->getWeaponClass();
				menuSprites().drawSpriteXYZ(pW->getSmallIconId(), 366 + i * 32, 308 + j * 32, 0, false, true);
                uint8 data[3];
                memset(data, 204, 3);
                if (pW->ammo() != -1) {
                    int n = wi->ammoRemaining();
                    if (pW->ammo() == 0)
                        n = 24;
                    else {
                        n *= 24;
                        n /= pW->ammo();
                    }
                    for (int k = 0; k < n; k++)
                        g_Screen.scale2x(366 + i * 32 + k + 4,
                                         308 + j * 32 + 22, 1, 3, data);
                }
            }
}

void SelectMenu::handleTick(int elapsed)
{
    tick_count_ += elapsed;
    // Updates the moving agent selector
    if (tick_count_ > 300) {
        rnd_ = ++rnd_ % 8;
        tick_count_ = 0;
        needRendering();
    }

    // Update the clock
    if (g_Session.updateTime(elapsed)) {
        updateClock();
    }
}

/*! 
 * Update the game time display
 */
void SelectMenu::updateClock() {
    char tmp[100];
    g_Session.getTimeAsStr(tmp);
    getStatic(txtTimeId_)->setText(tmp);

	// update money
    getStatic(moneyTxtId_)->setTextFormated("%d", g_Session.getMoney());
}

void SelectMenu::drawSelectedWeaponInfos(int x, int y) {
	char tmp[100];
	
    getMenuFont(FontManager::SIZE_1)->drawText(x, y, pSelectedWeap_->getName(), false);
	sprintf(tmp, "COST   :%d", pSelectedWeap_->cost());
    getMenuFont(FontManager::SIZE_1)->drawText(x, y + 12, tmp, false);
    y += 24;

    if (pSelectedWeap_->ammo() >= 0) {
		sprintf(tmp, "AMMO   :%d", pSelectedWeap_->ammo());
        getMenuFont(FontManager::SIZE_1)->drawText(x, y, tmp, false);
        y += 12;
    }

    if (pSelectedWeap_->range() >= 0) {
		sprintf(tmp, "RANGE  :%d", pSelectedWeap_->range());
        getMenuFont(FontManager::SIZE_1)->drawText(x, y, tmp, false);
        y += 12;
    }

    if (pSelectedWeap_->damagePerShot() >= 0 && pSelectedWeap_->ammo() >= 0) {
		sprintf(tmp, "SHOT   :%d", pSelectedWeap_->damagePerShot());
        getMenuFont(FontManager::SIZE_1)->drawText(x, y, tmp, false);
        y += 12;
    }

	if (selectedWInstId_ > 0 ) {
		WeaponInstance *wi = g_Session.teamMember(cur_agent_)->weapon(selectedWInstId_ - 1);
		if (pSelectedWeap_->ammo() > wi->ammoRemaining()) {
			int rldCost = (pSelectedWeap_->ammo()
									- wi->ammoRemaining()) * pSelectedWeap_->ammoCost();
		
			sprintf(tmp, "RELOAD :%d", rldCost);
			getMenuFont(FontManager::SIZE_1)->drawText(x, y, tmp, false);
			y += 12;
		}
    }
}

void SelectMenu::drawSelectedModInfos(int x, int y)
{
	getMenuFont(FontManager::SIZE_1)->drawText(x, y, pSelectedMod_->getName(), false);
    char tmp[100];
	sprintf(tmp, "COST   :%d", pSelectedMod_->cost());
    getMenuFont(FontManager::SIZE_1)->drawText(504, y + 14, tmp, false);
	getMenuFont(FontManager::SIZE_1)->drawText(504, y + 28, pSelectedMod_->desc(), false);
}

void SelectMenu::handleShow() {

    menu_manager_->saveBackground();

    // Show the mouse
    g_System.showCursor();

    // Update the time
    updateClock();

    if (g_Session.teamMember(cur_agent_)) {
        getStatic(txtAgentId_)->setTextFormated("#SELECT_SUBTITLE", g_Session.teamMember(cur_agent_)->getName());
    } else {
        getStatic(txtAgentId_)->setText("");
    }

    for (int iAgnt=0; iAgnt<AgentManager::MAX_AGENT; iAgnt++) {
        Agent *pAgentFromCryo = g_App.agents().agent(iAgnt);
        pTeamLBox_->setSquadLine(g_Session.getTeamSlot(pAgentFromCryo), iAgnt);
    }
}

void SelectMenu::handleRender(DirtyList &dirtyList) {
    g_Screen.drawLogo(18, 14, g_App.getGameSession().getLogo(), g_App.getGameSession().getLogoColour());

    // write team member icons and health
    uint8 data[4], datag[4];
    memset(data, 204, 4);
    memset(datag, 10, 4);
    Agent *t1 = g_Session.teamMember(0);
    Agent *t2 = g_Session.teamMember(1);
    Agent *t3 = g_Session.teamMember(2);
    Agent *t4 = g_Session.teamMember(3);
    if (t1) {
        if (t1->isActive()) {
            menuSprites().drawSpriteXYZ(Sprite::MSPR_SELECT_1, 20, 84, 0, false, true);
            for (int i = 0; i < t1->health() * 35 / 255; i++)
                g_Screen.scale2x(68, 122 - i, 3, 1, data);
        } else {
            for (int i = 0; i < t1->health() * 35 / 255; i++)
                g_Screen.scale2x(68, 122 - i, 3, 1, datag);
        }
    }
    if (t2) {
        if (t2->isActive()) {
            menuSprites().drawSpriteXYZ(Sprite::MSPR_SELECT_2, 82, 84, 0, false, true);
            for (int i = 0; i < t2->health() * 35 / 255; i++)
                g_Screen.scale2x(132, 122 - i, 3, 1, data);
        } else {
            for (int i = 0; i < t2->health() * 35 / 255; i++)
                g_Screen.scale2x(132, 122 - i, 3, 1, datag);
        }
    }
    if (t3) {
        if (t3->isActive()) {
            menuSprites().drawSpriteXYZ(Sprite::MSPR_SELECT_3, 20, 162, 0, false, true);
            for (int i = 0; i < t3->health() * 35 / 255; i++)
                g_Screen.scale2x(68, 200 - i, 3, 1, data);
        } else {
            for (int i = 0; i < t3->health() * 35 / 255; i++)
                g_Screen.scale2x(68, 200 - i, 3, 1, datag);
        }
    }
    if (t4) {
        if (t4->isActive()) {
            menuSprites().drawSpriteXYZ(Sprite::MSPR_SELECT_4, 82, 162, 0, false, true);
            for (int i = 0; i < t4->health() * 35 / 255; i++)
                g_Screen.scale2x(132, 200 - i, 3, 1, data);
        } else {
            for (int i = 0; i < t4->health() * 35 / 255; i++)
                g_Screen.scale2x(132, 200 - i, 3, 1, datag);
        }
    }
    if (sel_all_) {
        menuSprites().drawSpriteXYZ(77, 20, 152, 0, false, true);
    }

    // Draw the selector around the selected agent
    switch (cur_agent_) {
        case 0:
            drawAgentSelector(20, 84);
            break;
        case 1:
            drawAgentSelector(82, 84);
            break;
        case 2:
            drawAgentSelector(20, 162);
            break;
        case 3:
            drawAgentSelector(82, 162);
            break;
    }

    drawAgent();

    if (pSelectedWeap_) {
        uint8 ldata[62];
        memset(ldata, 16, sizeof(ldata));
        g_Screen.scale2x(502, 268, sizeof(ldata), 1, ldata);
        g_Screen.scale2x(502, 292, sizeof(ldata), 1, ldata);
        g_Screen.scale2x(502, 318, sizeof(ldata), 1, ldata);
        
		menuSprites().drawSpriteXYZ(pSelectedWeap_->getBigIconId(), 502, 106, 0, false, true);
        drawSelectedWeaponInfos(504, 194);
    
    } else if (pSelectedMod_) {
        uint8 ldata[62];
        memset(ldata, 16, sizeof(ldata));
        g_Screen.scale2x(502, 268, sizeof(ldata), 1, ldata);
        g_Screen.scale2x(502, 292, sizeof(ldata), 1, ldata);
        g_Screen.scale2x(502, 318, sizeof(ldata), 1, ldata);

        drawSelectedModInfos(504, 108);
    }
}

void SelectMenu::handleLeave() {
    g_System.hideCursor();
    // resetting menu
    tab_ = TAB_EQUIPS;
    showItemList();
    pSelectedWeap_ = NULL;
    selectedWInstId_ = 0;
    pSelectedMod_ = NULL;
    rnd_ = 0;
    cur_agent_ = 0;
    sel_all_ = false;
}

void SelectMenu::toggleAgent(int n)
{
    int nactive = 0;
    for (int i = 0; i < 4; i++)
        if (g_Session.teamMember(i) && g_Session.teamMember(i)->isActive())
            nactive++;
    Agent *a = g_Session.teamMember(n);
    if (a) {
        if (a->isActive() && nactive == 1)
            return;
        a->setActive(!a->isActive());
    }
}

bool SelectMenu::handleMouseDown(int x, int y, int button, const int modKeys)
{
    if (x >= 20 && x <= 140) {
        if (y >= 84 && y <= 150) {
            if (x >= 82) {
                if (button == 3)
                    toggleAgent(1);
                else {
                    cur_agent_ = 1;
                    if (g_Session.teamMember(1)) {
                        getStatic(txtAgentId_)->setTextFormated("#SELECT_SUBTITLE", g_Session.teamMember(1)->getName());
                    } else {
                        getStatic(txtAgentId_)->setText("");
                    }
                }
            } else {
                if (button == 3)
                    toggleAgent(0);
                else {
                    cur_agent_ = 0;
                    if (g_Session.teamMember(0)) {
                        getStatic(txtAgentId_)->setTextFormated("#SELECT_SUBTITLE", g_Session.teamMember(0)->getName());
                    } else {
                        getStatic(txtAgentId_)->setText("");
                    }
                }
            }
            needRendering();
        }
        if (y > 150 && y < 162) {
            sel_all_ = !sel_all_;
            needRendering();
        }
        if (y >= 162 && y <= 228) {
            if (x >= 82) {
                if (button == 3)
                    toggleAgent(3);
                else {
                    cur_agent_ = 3;
                    if (g_Session.teamMember(3)) {
                        getStatic(txtAgentId_)->setTextFormated("#SELECT_SUBTITLE", g_Session.teamMember(3)->getName());
                    } else {
                        getStatic(txtAgentId_)->setText("");
                    }
                }
            } else {
                if (button == 3)
                    toggleAgent(2);
                else {
                    cur_agent_ = 2;
                    if (g_Session.teamMember(2)) {
                        getStatic(txtAgentId_)->setTextFormated("#SELECT_SUBTITLE", g_Session.teamMember(2)->getName());
                    } else {
                        getStatic(txtAgentId_)->setText("");
                    }
                }
            }
            needRendering();
        }
    }

	// Checks if the user clicked on item in the current agent inventory
    Agent *selected = g_Session.teamMember(cur_agent_);
    if (selected) {
        for (int j = 0; j < 2; j++)
            for (int i = 0; i < 4; i++)
                if (j * 4 + i < selected->numWeapons() &&
                    x >= 366 + i * 32 && x < 366 + i * 32 + 32 &&
                    y >= 308 + j * 32 && y < 308 + j * 32 + 32)
                {
					// The user has actually selected a weapon from the inventory :
					// 1/ selects the EQUIPS toggle button
                    tab_ = TAB_EQUIPS;
                    pSelectedMod_ = NULL;
                    selectToggleAction(equipButId_);
					// 2/ computes the id of the selected weapon and selects it
					int newId = i + j * 4 + 1;

					if (newId != selectedWInstId_) { // Do something only if a different weapon is selected
						selectedWInstId_ = newId;
						WeaponInstance *wi = selected->weapon(selectedWInstId_ -1);
						pSelectedWeap_ = wi->getWeaponClass();
						addDirtyRect(500, 105,  125, 235);
						// 3/ see if reload button should be displayed
						bool displayReload = pSelectedWeap_->ammo() > wi->ammoRemaining();
						getOption(reloadButId_)->setVisible(displayReload);

						// 4/ hides the purchase button for the sell button
						getOption(purchaseButId_)->setVisible(false);
						getOption(cancelButId_)->setVisible(true);
						getOption(sellButId_)->setVisible(true);
						pTeamLBox_->setVisible(false);
						pModsLBox_->setVisible(false);
						pWeaponsLBox_->setVisible(false);
					}
                }
    }

	return false;
}

/*!
 * Hides the list of Mods or Weapon and shows the purchase and cancel buttons
 * that appear on the detail panel.
 */
void SelectMenu::showModWeaponPanel() {
	getOption(purchaseButId_)->setVisible(true);
	getOption(cancelButId_)->setVisible(true);
    if (tab_ == TAB_MODS) {
        pModsLBox_->setVisible(false);
    } else {
        pWeaponsLBox_->setVisible(false);
    }
}

void SelectMenu::showItemList() {
    addDirtyRect(500, 105,  125, 235);
    pSelectedMod_ = NULL;
    pSelectedWeap_ = NULL;
    selectedWInstId_ = 0;
    getOption(cancelButId_)->setVisible(false);
    getOption(reloadButId_)->setVisible(false);
    getOption(purchaseButId_)->setVisible(false);
    getOption(sellButId_)->setVisible(false);

    if (tab_ == TAB_MODS) {
        pModsLBox_->setVisible(true);
        pWeaponsLBox_->setVisible(false);
        pTeamLBox_->setVisible(false);
    } else if (tab_ == TAB_EQUIPS) {
        pModsLBox_->setVisible(false);
        pWeaponsLBox_->setVisible(true);
        pTeamLBox_->setVisible(false);
    } else {
        pModsLBox_->setVisible(false);
        pWeaponsLBox_->setVisible(false);
        pTeamLBox_->setVisible(true);
    }
}

void SelectMenu::handleAction(const int actionId, void *ctx, const int modKeys)
{
    if (actionId == teamButId_) {
        tab_ = TAB_TEAM;
        showItemList();
    } else if (actionId == modsButId_) {
        tab_ = TAB_MODS;
        showItemList();
    } else if (actionId == equipButId_) {
        tab_ = TAB_EQUIPS;
        showItemList();
    } else if (actionId == pTeamLBox_->getId()) {
        // get the selected agent from the team listbox
        std::pair<int, void *> * pPair = static_cast<std::pair<int, void *> *> (ctx);
        Agent *pNewAgent = static_cast<Agent *> (pPair->second);
        
        bool found = false;
        int j;
        // check if selected agent is already part of the mission squad
        for (j = 0; j < 4; j++) {
            if (g_Session.teamMember(j) == pNewAgent) {
                found = true;
                break;
            }
        }

        // Agent was not part of the squad
        if (!found) {
            // adds his to the squad
            g_Session.setTeamMember(cur_agent_, pNewAgent);
            // Update current agent name
            getStatic(txtAgentId_)->setTextFormated("#SELECT_SUBTITLE", pNewAgent->getName());
            pTeamLBox_->setSquadLine(cur_agent_, pPair->first);
        } else if (j == cur_agent_){
            // else if agent is the currently selected
            // removes him from squad
            g_Session.setTeamMember(j, NULL);
            getStatic(txtAgentId_)->setTextFormated("");
            pTeamLBox_->setSquadLine(cur_agent_, -1);
        }
        // redraw agent display
        addDirtyRect(158, 110, 340, 260);
        // redraw agent buttons
        addDirtyRect(16, 80, 130, 155);
        
    } else if (actionId == pModsLBox_->getId()) {
        std::pair<int, void *> * pPair = static_cast<std::pair<int, void *> *> (ctx);
        pSelectedMod_ = static_cast<Mod *> (pPair->second);
        showModWeaponPanel();
    } else if (actionId == pWeaponsLBox_->getId()) {
        std::pair<int, void *> * pPair = static_cast<std::pair<int, void *> *> (ctx);
        pSelectedWeap_ = static_cast<Weapon *> (pPair->second);
        showModWeaponPanel();
    } else if (actionId == cancelButId_) {
        showItemList();
    } else if (actionId == reloadButId_) {
        Agent *selected = g_Session.teamMember(cur_agent_);
        WeaponInstance *wi = selected->weapon(selectedWInstId_ - 1);
		int rldCost = (pSelectedWeap_->ammo()
						- wi->ammoRemaining()) * pSelectedWeap_->ammoCost();

        if (g_Session.getMoney() >= rldCost) {
            g_Session.setMoney(g_Session.getMoney() - rldCost);
            wi->setAmmoRemaining(pSelectedWeap_->ammo());
            getOption(reloadButId_)->setVisible(false);
			getStatic(moneyTxtId_)->setTextFormated("%d", g_Session.getMoney());
        }
    } else if (actionId == purchaseButId_) {
        // Buying weapon
        if (pSelectedWeap_) {
            if (sel_all_) {
                for (int n = 0; n < 4; n++) {
                    Agent *selected = g_Session.teamMember(n);
                    if (selected && selected->numWeapons() < 8
                        && g_Session.getMoney() >= pSelectedWeap_->cost()) {
                        g_Session.setMoney(g_Session.getMoney() - pSelectedWeap_->cost());
                        selected->addWeapon(pSelectedWeap_->createInstance());
						getStatic(moneyTxtId_)->setTextFormated("%d", g_Session.getMoney());
                    }
                }
            } else {
                Agent *selected = g_Session.teamMember(cur_agent_);
                if (selected && selected->numWeapons() < 8
                    && g_Session.getMoney() >= pSelectedWeap_->cost()) {
                    g_Session.setMoney(g_Session.getMoney() - pSelectedWeap_->cost());
                    selected->addWeapon(pSelectedWeap_->createInstance());
					getStatic(moneyTxtId_)->setTextFormated("%d", g_Session.getMoney());
                }
            }
            needRendering();
        } else if (pSelectedMod_) {
            if (sel_all_) {
                for (int n = 0; n < 4; n++) {
                    Agent *selected = g_Session.teamMember(n);
                    if (selected && selected->canHaveMod(pSelectedMod_)
                        && g_Session.getMoney() >= pSelectedMod_->cost()) {
                        selected->addMod(pSelectedMod_);
                        g_Session.setMoney(g_Session.getMoney() - pSelectedMod_->cost());
						getStatic(moneyTxtId_)->setTextFormated("%d", g_Session.getMoney());
                    }
                }
            } else {
                Agent *selected = g_Session.teamMember(cur_agent_);
                if (selected && selected->canHaveMod(pSelectedMod_)
                    && g_Session.getMoney() >= pSelectedMod_->cost()) {
                    selected->addMod(pSelectedMod_);
                    g_Session.setMoney(g_Session.getMoney() - pSelectedMod_->cost());
					getStatic(moneyTxtId_)->setTextFormated("%d", g_Session.getMoney());
                }
            }
            showItemList();
        }
    
    } else if (actionId == sellButId_ && selectedWInstId_) {
        addDirtyRect(360, 305, 135, 70);
        Agent *selected = g_Session.teamMember(cur_agent_);
        WeaponInstance *pWi = selected->removeWeapon(selectedWInstId_ - 1);
        g_Session.setMoney(g_Session.getMoney() + pWi->getWeaponClass()->cost());
		getStatic(moneyTxtId_)->setTextFormated("%d", g_Session.getMoney());
        delete pWi;
        showItemList();
    }
}
