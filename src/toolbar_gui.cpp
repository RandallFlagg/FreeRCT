/*
 * This file is part of FreeRCT.
 * FreeRCT is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * FreeRCT is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with FreeRCT. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file toolbar_gui.cpp Main toolbar window code. */

#include "stdafx.h"
#include "freerct.h"
#include "window.h"
#include "video.h"
#include "language.h"
#include "finances.h"
#include "dates.h"
#include "math_func.h"
#include "sprite_store.h"
#include "gui_sprites.h"
#include "messages.h"
#include "person.h"
#include "people.h"
#include "viewport.h"
#include "gamecontrol.h"
#include "weather.h"

/**
 * Top toolbar.
 * @ingroup gui_group
 */
class ToolbarWindow : public GuiWindow {
public:
	ToolbarWindow();

	Point32 OnInitialPosition() override;
	void OnClick(WidgetNumber number, const Point16 &pos) override;
	void OnChange(ChangeCode code, uint32 parameter) override;
	void SetWidgetStringParameters(WidgetNumber wid_num) const override;
	void UpdateWidgetSize(WidgetNumber wid_num, BaseWidget *wid) override;
};

/**
 * Widget numbers of the toolbar GUI.
 * @ingroup gui_group
 */
enum ToolbarGuiWidgets {
	TB_DROPDOWN_MAIN,     ///< Main menu dropdown.
	TB_DROPDOWN_SPEED,    ///< Game speed dropdown.
	TB_DROPDOWN_VIEW,     ///< View options dropdown.
	TB_GUI_PATHS,         ///< Build paths button.
	TB_GUI_RIDE_SELECT,   ///< Select ride button.
	TB_GUI_FENCE,         ///< Select fence button.
	TB_GUI_SCENERY,       ///< Select scenery button.
	TB_GUI_PATH_OBJECTS,  ///< Select path objects button.
	TB_GUI_TERRAFORM,     ///< Terraform button.
	TB_GUI_FINANCES,      ///< Finances button.
	TB_GUI_STAFF,         ///< Staff button.
	TB_GUI_INBOX,         ///< Inbox button.
};

/**
 * Entries in the main menu dropdown.
 * @ingroup gui_group
 */
enum DropdownMain {
	DDM_SAVE,      ///< Save game.
	DDM_GAME_MODE, ///< Switch game mode.
	DDM_SETTINGS,  ///< General settings.
	DDM_MENU,      ///< Back to main menu.
	DDM_QUIT,      ///< Quit the game.
	DDM_COUNT      ///< Number of entries.
};

/**
 * Entries in the view options dropdown.
 * @ingroup gui_group
 */
enum DropdownView {
	DDV_MINIMAP,      ///< Open the minimap.
	DDV_UNDERGROUND,  ///< Toggle underground view.
};

/**
 * Widget parts of the toolbar GUI.
 * @ingroup gui_group
 */
static const WidgetPart _toolbar_widgets[] = {
	Intermediate(1, 0),
		Widget(WT_IMAGE_DROPDOWN_BUTTON, TB_DROPDOWN_MAIN,   COL_RANGE_ORANGE_BROWN), SetData(SPR_GUI_TOOLBAR_MAIN,  GUI_TOOLBAR_GUI_DROPDOWN_MAIN),
		Widget(WT_IMAGE_DROPDOWN_BUTTON, TB_DROPDOWN_SPEED,  COL_RANGE_ORANGE_BROWN), SetData(SPR_GUI_TOOLBAR_SPEED, GUI_TOOLBAR_GUI_DROPDOWN_SPEED_TOOLTIP),
		Widget(WT_IMAGE_DROPDOWN_BUTTON, TB_DROPDOWN_VIEW,   COL_RANGE_ORANGE_BROWN), SetData(SPR_GUI_TOOLBAR_VIEW,  GUI_TOOLBAR_GUI_DROPDOWN_VIEW_TOOLTIP),
		Widget(WT_EMPTY, INVALID_WIDGET_INDEX, COL_RANGE_ORANGE_BROWN), SetMinimalSize(16, 16),
		Widget(WT_IMAGE_PUSHBUTTON, TB_GUI_TERRAFORM,    COL_RANGE_ORANGE_BROWN), SetData(SPR_GUI_TOOLBAR_TERRAIN, GUI_TOOLBAR_GUI_TOOLTIP_TERRAFORM),
		Widget(WT_IMAGE_PUSHBUTTON, TB_GUI_PATHS,        COL_RANGE_ORANGE_BROWN), SetData(SPR_GUI_TOOLBAR_PATH,    GUI_TOOLBAR_GUI_TOOLTIP_BUILD_PATHS),
		Widget(WT_IMAGE_PUSHBUTTON, TB_GUI_FENCE,        COL_RANGE_ORANGE_BROWN), SetData(SPR_GUI_TOOLBAR_FENCE,   GUI_TOOLBAR_GUI_TOOLTIP_FENCE),
		Widget(WT_IMAGE_PUSHBUTTON, TB_GUI_SCENERY,      COL_RANGE_ORANGE_BROWN), SetData(SPR_GUI_TOOLBAR_SCENERY, GUI_TOOLBAR_GUI_TOOLTIP_SCENERY),
		Widget(WT_IMAGE_PUSHBUTTON, TB_GUI_PATH_OBJECTS, COL_RANGE_ORANGE_BROWN), SetData(SPR_GUI_TOOLBAR_OBJECTS, GUI_TOOLBAR_GUI_TOOLTIP_PATH_OBJECTS),
		Widget(WT_IMAGE_PUSHBUTTON, TB_GUI_RIDE_SELECT,  COL_RANGE_ORANGE_BROWN), SetData(SPR_GUI_TOOLBAR_RIDE,    GUI_TOOLBAR_GUI_TOOLTIP_RIDE_SELECT),
		Widget(WT_EMPTY, INVALID_WIDGET_INDEX, COL_RANGE_ORANGE_BROWN), SetMinimalSize(16, 16),
		Widget(WT_IMAGE_PUSHBUTTON, TB_GUI_STAFF,       COL_RANGE_ORANGE_BROWN), SetData(SPR_GUI_TOOLBAR_STAFF,    GUI_TOOLBAR_GUI_TOOLTIP_STAFF),
		Widget(WT_IMAGE_PUSHBUTTON, TB_GUI_INBOX,       COL_RANGE_ORANGE_BROWN), SetData(SPR_GUI_TOOLBAR_INBOX,    GUI_TOOLBAR_GUI_TOOLTIP_INBOX),
		Widget(WT_IMAGE_PUSHBUTTON, TB_GUI_FINANCES,    COL_RANGE_ORANGE_BROWN), SetData(SPR_GUI_TOOLBAR_FINANCES, GUI_TOOLBAR_GUI_TOOLTIP_FINANCES),
	EndContainer(),
};

ToolbarWindow::ToolbarWindow() : GuiWindow(WC_TOOLBAR, ALL_WINDOWS_OF_TYPE)
{
	this->closeable = false;
	this->SetupWidgetTree(_toolbar_widgets, lengthof(_toolbar_widgets));
}

Point32 ToolbarWindow::OnInitialPosition()
{
	static const Point32 pt(10, 0);
	return pt;
}

/**
 * Determines the string ID of the string to display
 * for the switch game mode button.
 * @return String id of the string to display.
 */
StringID GetSwitchGameModeString()
{
	switch (_game_mode_mgr.GetGameMode()) {
		case GM_PLAY:
			return GUI_TOOLBAR_GUI_GAME_MODE_EDITOR;

		case GM_EDITOR:
			return GUI_TOOLBAR_GUI_GAME_MODE_PLAY;

		case GM_NONE:
			/* The toolbar is not visible in none-mode. */
			return STR_NULL;

		default:
			NOT_REACHED();
	}
}

void ToolbarWindow::OnClick(WidgetNumber number, const Point16 &pos)
{
	switch (number) {
		case TB_DROPDOWN_MAIN: {
			DropdownList itemlist;
			for (int i = 0; i < DDM_COUNT; i++) {
				_str_params.SetStrID(1, i == DDM_GAME_MODE ? GetSwitchGameModeString() : GUI_MAIN_MENU_SAVE + i);
				itemlist.push_back(DropdownItem(STR_ARG1));
			}
			this->ShowDropdownMenu(number, itemlist, -1);
			break;
		}
		case TB_DROPDOWN_SPEED: {
			DropdownList itemlist;
			for (int i = 0; i < GSP_COUNT; i++) {
				_str_params.SetStrID(1, GUI_TOOLBAR_GUI_DROPDOWN_SPEED_PAUSE + i);
				itemlist.push_back(DropdownItem(STR_ARG1));
			}
			this->ShowDropdownMenu(number, itemlist, _game_control.speed);
			break;
		}
		case TB_DROPDOWN_VIEW: {
			DropdownList itemlist;
			/* Keep the order consistent with the DropdownView ordering! */
			/* DDV_MINIMAP */
			itemlist.push_back(DropdownItem(GUI_TOOLBAR_GUI_DROPDOWN_VIEW_MINIMAP));
			/* DDV_UNDERGROUND */
			_str_params.SetStrID(1, GUI_TOOLBAR_GUI_DROPDOWN_VIEW_UNDERGROUND);
			itemlist.push_back(DropdownItem(_window_manager.GetViewport()->underground_mode ? GUI_DROPDOWN_CHECKED : GUI_DROPDOWN_UNCHECKED));

			this->ShowDropdownMenu(number, itemlist, -1);
			break;
		}

		case TB_GUI_PATHS:
			ShowPathBuildGui();
			break;

		case TB_GUI_RIDE_SELECT:
			ShowRideSelectGui();
			break;

		case TB_GUI_FENCE:
			ShowFenceGui();
			break;

		case TB_GUI_SCENERY:
			ShowSceneryGui();
			break;

		case TB_GUI_PATH_OBJECTS:
			ShowPathObjectsGui();
			break;

		case TB_GUI_TERRAFORM:
			ShowTerraformGui();
			break;

		case TB_GUI_FINANCES:
			ShowFinancesGui();
			break;

		case TB_GUI_STAFF:
			ShowStaffManagementGui();
			break;

		case TB_GUI_INBOX:
			ShowInboxGui();
			break;
	}
}

void ToolbarWindow::OnChange(ChangeCode code, uint32 parameter)
{
	switch (code) {
		case CHG_DROPDOWN_RESULT: {
			const int entry = parameter & 0xFF;
			switch ((parameter >> 16) & 0xFF) {
				case TB_DROPDOWN_MAIN:
					switch (entry) {
						case DDM_QUIT:
							ShowQuitProgram(false);
							break;
						case DDM_SETTINGS:
							ShowSettingGui();
							break;
						case DDM_GAME_MODE:
							_game_mode_mgr.SetGameMode(_game_mode_mgr.InEditorMode() ? GM_PLAY : GM_EDITOR);
							break;
						case DDM_SAVE:
							ShowSaveGameGui();
							break;
						case DDM_MENU:
							ShowQuitProgram(true);
							break;
						default: NOT_REACHED();
					}
					break;
				case TB_DROPDOWN_SPEED:
					_game_control.speed = static_cast<GameSpeed>(entry);
					break;
				case TB_DROPDOWN_VIEW:
					switch (entry) {
						case DDV_MINIMAP:
							ShowMinimap();
							break;
						case DDV_UNDERGROUND:
							_window_manager.GetViewport()->ToggleUndergroundMode();
							break;
						default: NOT_REACHED();
					}
					break;
			}
			break;
		}

		default:
			break;
	}
}

void ToolbarWindow::UpdateWidgetSize(WidgetNumber wid_num, BaseWidget *wid)
{
}

void ToolbarWindow::SetWidgetStringParameters(WidgetNumber wid_num) const
{
}

/**
 * Open the main toolbar window.
 * @ingroup gui_group
 */
void ShowToolbar()
{
	new ToolbarWindow();
}

/**
 * Bottom toolbar.
 * @ingroup gui_group
 */
class BottomToolbarWindow : public GuiWindow {
public:
	BottomToolbarWindow();

	Point32 OnInitialPosition() override;
	void SetWidgetStringParameters(WidgetNumber wid_num) const override;
	void OnChange(ChangeCode code, uint32 parameter) override;
	void UpdateWidgetSize(WidgetNumber wid_num, BaseWidget *wid) override;
	void DrawWidget(WidgetNumber wid_num, const BaseWidget *wid) const override;
	void OnClick(const WidgetNumber wid_num, const Point16 &pos) override;

private:
	int32 guest_count; ///< Number of guests in the park.
};

/**
 * Widget numbers of the bottom toolbar GUI.
 * @ingroup gui_group
 */
enum BottomToolbarGuiWidgets {
	BTB_EMPTY,          ///< Empty widget defining the width of the status bar.
	BTB_STATUS,         ///< Status panel containing cash and rating readout.
	BTB_WEATHER,        ///< Weather sprite.
	BTB_TEMPERATURE,    ///< Temperature in the park.
	BTB_MESSAGE,        ///< A preview of the last message.
	BTB_VIEW_DIRECTION, ///< Status panel containing viewing direction.
	BTB_GUESTCOUNT,     ///< Display of number of guests in the park.
	BTB_DATE,           ///< Status panel containing date.
};

static const uint32 BOTTOM_BAR_HEIGHT = 35;     ///< Minimum Y-coord size of the bottom toolbar (BTB) panel.
static const uint32 BOTTOM_BAR_POSITION_X = 75; ///< Separation of the toolbar from the edge of the window.

/**
 * Widget parts of the bottom toolbar GUI.
 * @ingroup gui_group
 * @todo Left/Right Padding get ignored when drawing text widgets
 * @todo Implement non-minimal default window size to prevent the need to compute remaining space manually.
 */
static const WidgetPart _bottom_toolbar_widgets[] = {
	Intermediate(0, 1),
		Widget(WT_EMPTY, BTB_EMPTY, COL_RANGE_INVALID),
		Widget(WT_PANEL, INVALID_WIDGET_INDEX, COL_RANGE_ORANGE_BROWN),
			Intermediate(1, 0), SetPadding(0, 3, 0, 3),
				Intermediate(3, 1),
					Widget(WT_LEFT_TEXT, BTB_STATUS, COL_RANGE_ORANGE_BROWN), SetData(STR_ARG1, STR_NULL), SetFill(0, 0),
					Widget(WT_LEFT_TEXT, BTB_GUESTCOUNT, COL_RANGE_ORANGE_BROWN), SetData(GUI_BOTTOMBAR_GUESTCOUNT, STR_NULL), SetFill(0, 0),
					Widget(WT_EMPTY, INVALID_WIDGET_INDEX, COL_RANGE_INVALID), SetFill(0, 1),
				Widget(WT_EMPTY, BTB_WEATHER, COL_RANGE_ORANGE_BROWN), SetPadding(3, 3, 3, 3), SetFill(0, 1),
				Widget(WT_RIGHT_TEXT, BTB_TEMPERATURE, COL_RANGE_ORANGE_BROWN), SetFill(0, 0), SetData(STR_ARG1, STR_NULL),
				Widget(WT_EMPTY, INVALID_WIDGET_INDEX, COL_RANGE_ORANGE_BROWN), SetMinimalSize(1, BOTTOM_BAR_HEIGHT), SetFill(1, 0),
				Widget(WT_EMPTY, BTB_MESSAGE, COL_RANGE_ORANGE_BROWN), SetFill(1, 0), SetMinimalSize(300, BOTTOM_BAR_HEIGHT), 
				Widget(WT_EMPTY, INVALID_WIDGET_INDEX, COL_RANGE_ORANGE_BROWN), SetMinimalSize(1, BOTTOM_BAR_HEIGHT), SetFill(1, 0),
				Widget(WT_EMPTY, BTB_VIEW_DIRECTION, COL_RANGE_ORANGE_BROWN), SetMinimalSize(1, BOTTOM_BAR_HEIGHT), SetFill(0, 0),
				Widget(WT_RIGHT_TEXT, BTB_DATE, COL_RANGE_ORANGE_BROWN), SetPadding(3, 0, 30, 0), SetData(STR_ARG1, STR_NULL),
						SetMinimalSize(1, BOTTOM_BAR_HEIGHT), SetFill(0, 0),
			EndContainer(),
	EndContainer(),
};

BottomToolbarWindow::BottomToolbarWindow() : GuiWindow(WC_BOTTOM_TOOLBAR, ALL_WINDOWS_OF_TYPE)
{
	this->closeable = false;
	this->guest_count = _guests.CountGuestsInPark();
	this->SetupWidgetTree(_bottom_toolbar_widgets, lengthof(_bottom_toolbar_widgets));
}

Point32 BottomToolbarWindow::OnInitialPosition()
{
	static Point32 pt;
	pt.x = BOTTOM_BAR_POSITION_X;
	pt.y = _video.GetYSize() - BOTTOM_BAR_HEIGHT;
	return pt;
}

void BottomToolbarWindow::SetWidgetStringParameters(WidgetNumber wid_num) const
{
	switch (wid_num) {
		case BTB_STATUS:
			_finances_manager.CashToStrParams();
			break;

		case BTB_DATE:
			_str_params.SetDate(1, _date);
			break;

		case BTB_TEMPERATURE:
			_str_params.SetTemperature(1, _weather.temperature);
			break;

		case BTB_GUESTCOUNT:
			_str_params.SetNumber(1, this->guest_count);
			break;
	}
}

void BottomToolbarWindow::OnClick(const WidgetNumber wid_num, const Point16 &pos)
{
	if (wid_num != BTB_MESSAGE || _inbox.display_message == nullptr) return GuiWindow::OnClick(wid_num, pos);

	BaseWidget *w = this->GetWidget<BaseWidget>(wid_num);
	if (pos.x < w->pos.width - w->pos.height) {
		_inbox.DismissDisplayMessage();
	} else {
		_inbox.display_message->OnClick();
	}
}

void BottomToolbarWindow::OnChange(ChangeCode code, uint32 parameter)
{
	switch (code) {
		case CHG_DISPLAY_OLD:
			this->MarkDirty();
			break;

		case CHG_RESOLUTION_CHANGED:
			this->ResetSize();
			break;

		case CHG_GUEST_COUNT:
			/* Parameter decides meaning.
			 * - 0 means a guest left.
			 * - 1 means a guest entered.
			 * - otherwise, recount.
			 */
			if (parameter == 0) {
				this->guest_count = std::max(0, this->guest_count - 1);
			} else if (parameter == 1) {
				this->guest_count++;
			} else {
				this->guest_count = _guests.CountGuestsInPark();
			}
			this->MarkDirty();
			break;

		default:
			break; // Ignore other messages.
	}
}

void BottomToolbarWindow::UpdateWidgetSize(WidgetNumber wid_num, BaseWidget *wid)
{
	/* -99,999,999.99 = Maximum amount of money that is worth handling for now. */
	static const int64 LARGE_MONEY_AMOUNT = -9999999999;
	static const int LARGE_TEMPERATURE = 999; // Large enough to format all temperatures.
	static const int MANY_GUESTS = 10000;
	Point32 p(0, 0);

	switch (wid_num) {
		case BTB_STATUS:
			p = GetMoneyStringSize(LARGE_MONEY_AMOUNT);
			break;

		case BTB_VIEW_DIRECTION: {
			Rectangle16 rect = _sprite_manager.GetTableSpriteSize(SPR_GUI_COMPASS_START); // It's the same size for all compass sprites.
			p = {rect.width, rect.height};
			break;
		}

		case BTB_WEATHER: {
			Rectangle16 rect = _sprite_manager.GetTableSpriteSize(SPR_GUI_WEATHER_START);
			p = {rect.width, rect.height};
			break;
		}

		case BTB_TEMPERATURE:
			_str_params.SetTemperature(1, LARGE_TEMPERATURE);
			GetTextSize(STR_ARG1, &p.x, &p.y);
			break;

		case BTB_GUESTCOUNT:
			_str_params.SetNumber(1, MANY_GUESTS);
			GetTextSize(GUI_BOTTOMBAR_GUESTCOUNT, &p.x, &p.y);
			break;

		case BTB_EMPTY:
			p.x = _video.GetXSize() - (2 * BOTTOM_BAR_POSITION_X);
			break;

		case BTB_DATE:
			p = GetMaxDateSize();
			break;
	}

	wid->min_x = std::max(wid->min_x, (uint16)p.x);
	wid->min_y = std::max(wid->min_y, (uint16)p.y);
}

void BottomToolbarWindow::DrawWidget(WidgetNumber wid_num, const BaseWidget *wid) const
{
	static Recolouring recolour; // Never changed.

	switch (wid_num) {
		case BTB_VIEW_DIRECTION: {
			Viewport *vp = _window_manager.GetViewport();
			int dir = (vp == nullptr) ? 0 : vp->orientation;
			const ImageData *img = _sprite_manager.GetTableSprite(SPR_GUI_COMPASS_START + dir);
			if (img != nullptr) _video.BlitImage({GetWidgetScreenX(wid), GetWidgetScreenY(wid)}, img, recolour, GS_NORMAL);
			break;
		}

		case BTB_WEATHER: {
			int spr = SPR_GUI_WEATHER_START + _weather.GetWeatherType();
			const ImageData *img = _sprite_manager.GetTableSprite(spr);
			if (img != nullptr) _video.BlitImage({GetWidgetScreenX(wid), GetWidgetScreenY(wid)}, img, recolour, GS_NORMAL);
			break;
		}

		case BTB_MESSAGE:
			if (_inbox.display_message != nullptr) {
				DrawMessage(_inbox.display_message, Rectangle32(this->GetWidgetScreenX(wid), this->GetWidgetScreenY(wid), wid->pos.width, wid->pos.height), true);
			}
			break;
	}
}

/**
 * Open the bottom toolbar window.
 * @ingroup gui_group
 */
void ShowBottomToolbar()
{
	new BottomToolbarWindow();
}

/** %Window to ask confirmation for ending the program. */
class QuitProgramWindow : public GuiWindow {
public:
	explicit QuitProgramWindow(bool back_to_main_menu);

	Point32 OnInitialPosition() override;
	void OnClick(WidgetNumber number, const Point16 &pos) override;
	bool OnKeyEvent(WmKeyCode key_code, WmKeyMod mod, const std::string &symbol) override;

private:
	bool back_to_main_menu;  ///< Just return to the main menu instead of quitting FreeRCT.
};

/**
 * Widget numbers of the quit-program window.
 * @ingroup gui_group
 */
enum QuitProgramWidgets {
	QP_MESSAGE, ///< Displayed message.
	QP_YES,     ///< 'yes' button.
	QP_NO,      ///< 'no' button.
};

/** Window definition of the quit program GUI. */
#define QUIT_PROGRAM_WIDGET_TREE(caption, message) \
const WidgetPart _quit_program_widgets[] = { \
	Intermediate(0, 1), \
		Intermediate(1, 0), \
			Widget(WT_TITLEBAR, INVALID_WIDGET_INDEX, COL_RANGE_RED), SetData(caption, GUI_TITLEBAR_TIP), \
			Widget(WT_CLOSEBOX, INVALID_WIDGET_INDEX, COL_RANGE_RED), \
		EndContainer(), \
		Widget(WT_PANEL, INVALID_WIDGET_INDEX, COL_RANGE_RED), \
			Intermediate(2,0), \
				Widget(WT_CENTERED_TEXT, QP_MESSAGE, COL_RANGE_RED), \
						SetData(message, STR_NULL), SetPadding(5, 5, 5, 5), \
			EndContainer(), \
			Intermediate(1, 5), SetPadding(0, 0, 3, 0), \
				Widget(WT_EMPTY, INVALID_WIDGET_INDEX, COL_RANGE_INVALID), SetFill(1, 0), \
				Widget(WT_TEXT_PUSHBUTTON, QP_NO, COL_RANGE_YELLOW), SetData(GUI_QUIT_NO, STR_NULL), \
				Widget(WT_EMPTY, INVALID_WIDGET_INDEX, COL_RANGE_INVALID), SetFill(1, 0), \
				Widget(WT_TEXT_PUSHBUTTON, QP_YES, COL_RANGE_YELLOW), SetData(GUI_QUIT_YES, STR_NULL), \
				Widget(WT_EMPTY, INVALID_WIDGET_INDEX, COL_RANGE_INVALID), SetFill(1, 0), \
			EndContainer(), \
}

Point32 QuitProgramWindow::OnInitialPosition()
{
	Point32 pt;
	pt.x = (_video.GetXSize() - this->rect.width ) / 2;
	pt.y = (_video.GetYSize() - this->rect.height) / 2;
	return pt;
}

/**
 * Constructor.
 * @param b Just return to the main menu instead of quitting FreeRCT.
 */
QuitProgramWindow::QuitProgramWindow(bool b) : GuiWindow(WC_QUIT, ALL_WINDOWS_OF_TYPE)
{
	this->back_to_main_menu = b;
	QUIT_PROGRAM_WIDGET_TREE(this->back_to_main_menu ? GUI_RETURN_CAPTION : GUI_QUIT_CAPTION, this->back_to_main_menu ? GUI_RETURN_MESSAGE : GUI_QUIT_MESSAGE);
	this->SetupWidgetTree(_quit_program_widgets, lengthof(_quit_program_widgets));
}

bool QuitProgramWindow::OnKeyEvent(WmKeyCode key_code, WmKeyMod mod, const std::string &symbol)
{
	if (key_code == WMKC_CONFIRM) {
		this->OnClick(QP_YES, Point16());
		return true;
	}
	return GuiWindow::OnKeyEvent(key_code, mod, symbol);
}

void QuitProgramWindow::OnClick(WidgetNumber number, const Point16 &pos)
{
	if (number == QP_YES) {
		if (this->back_to_main_menu) {
			_game_control.MainMenu();
		} else {
			_game_control.QuitGame();
		}
	} else if (number != QP_NO) {
		return;
	}
	delete this;
}

/**
 * Ask the user whether the game should be stopped.
 * @param back_to_main_menu Just return to the main menu instead of quitting FreeRCT.
 */
void ShowQuitProgram(const bool back_to_main_menu)
{
	Window *w = GetWindowByType(WC_QUIT, ALL_WINDOWS_OF_TYPE);
	delete w;

	new QuitProgramWindow(back_to_main_menu);
}
