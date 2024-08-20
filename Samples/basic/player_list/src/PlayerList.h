/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 * Copyright (c) 2019-2024 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef RMLUI_SAMPLES_PLAYERLIST_PLAYERLIST_H
#define RMLUI_SAMPLES_PLAYERLIST_PLAYERLIST_H

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/Types.h>

namespace Rml {
class Context;
class Element;
class ElementDocument;
} // namespace Rml

struct PlayerEntry {
	bool is_local;
	uint32_t entity_id;

	uint32_t score;

	uint16_t latency;

	bool is_muted;
	bool is_friend;

	Rml::String name;

	Rml::String tag_name;
	Rml::Colourb tag_color;
};

PlayerEntry GenerateFakePlayerEntry(bool is_local);

class PlayerList final : public Rml::EventListener {
public:
	PlayerList(Rml::Context* context);

	void Update(double t);
	void Shutdown();

	void ProcessEvent(Rml::Event& event) override;

	Rml::ElementDocument* GetDocument() const;

	void AddPlayer(PlayerEntry&& player);

private:
	void InitializeDataModel(Rml::Context* context);

	void Filter();
	void Sort();

	void OnScrollUpdate();

	void UpdateDisplayedRows();

private:
	Rml::ElementDocument* document;
	Rml::DataModelHandle data_model;

	Rml::Element* players_element;
	Rml::Element* top_filler_element;
	Rml::Element* bottom_filler_element;

	double last_update;

	// A collection of all player data from the players manager.
	Rml::Vector<PlayerEntry> data;
	// A collection of players that is used in the UI document itself. It may be sorted, filtered out, ...
	Rml::Vector<PlayerEntry> entries;

	Rml::Vector2i data_range;

	Rml::String search_query;
};

#endif
