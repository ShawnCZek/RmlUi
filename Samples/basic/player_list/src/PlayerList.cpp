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

#include "PlayerList.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Profiling.h>
#include <RmlUi_Backend.h>

PlayerList::PlayerList(Rml::Context* context) : last_update(0)
{
	InitializeDataModel(context);

	document = context->LoadDocument("basic/player_list/data/player_list.rml");

	if (document)
	{
		document->Show();

		players_element = document->GetElementById("players");
		top_filler_element = players_element->QuerySelector(".filler_top");
		bottom_filler_element = players_element->QuerySelector(".filler_bottom");

		RMLUI_ASSERT(players_element);
		RMLUI_ASSERT(top_filler_element);
		RMLUI_ASSERT(bottom_filler_element);
	}
	else
	{
		players_element = nullptr;
		top_filler_element = nullptr;
		bottom_filler_element = nullptr;
	}
}

void PlayerList::Update(double t)
{
	if (t < last_update + 1.0)
		return;

	RMLUI_ZoneScoped;

	data.erase(data.begin() + 1);
	data.push_back(GenerateFakePlayerEntry(false));
	data.push_back(GenerateFakePlayerEntry(false));

	for (PlayerEntry& player : data)
	{
		player.score = (uint32_t)Rml::Math::RandomInteger(100);
		player.latency = (uint16_t)Rml::Math::RandomInteger(500);
	}

	UpdateDisplayedRows();

	last_update = t;
}

void PlayerList::Shutdown()
{
	bottom_filler_element = nullptr;
	top_filler_element = nullptr;
	players_element = nullptr;

	if (document)
	{
		document->Close();
		document = nullptr;
	}
}

void PlayerList::ProcessEvent(Rml::Event& event)
{
	switch (event.GetId())
	{
	case Rml::EventId::Keydown:
	{
		if (event.GetParameter<int>("key_identifier", 0) == Rml::Input::KI_ESCAPE)
			Backend::RequestExit();
	}
	break;
	case Rml::EventId::Scroll: OnScrollUpdate(); break;
	default: break;
	}
}

Rml::ElementDocument* PlayerList::GetDocument() const
{
	return document;
}

void PlayerList::AddPlayer(PlayerEntry&& player)
{
	data.emplace_back(std::move(player));
}

static void InitializePlayerEntryDataModel(Rml::DataModelConstructor& constructor)
{
	if (auto player = constructor.RegisterStruct<PlayerEntry>())
	{
		player.RegisterMember("id", &PlayerEntry::entity_id);
		player.RegisterMember("score", &PlayerEntry::score);
		player.RegisterMember("latency", &PlayerEntry::latency);
		player.RegisterMember("is_muted", &PlayerEntry::is_muted);
		player.RegisterMember("is_friend", &PlayerEntry::is_friend);
		player.RegisterMember("name", &PlayerEntry::name);
		player.RegisterMember("tag_name", &PlayerEntry::tag_name);
		player.RegisterMember("tag_color", &PlayerEntry::tag_color);
	}
}

void PlayerList::InitializeDataModel(Rml::Context* context)
{
	if (Rml::DataModelConstructor constructor = context->CreateDataModel("player_list"))
	{
		constructor.RegisterScalar<Rml::Colourb>([](const Rml::Colourb& c, Rml::Variant& variant) {
			variant = Rml::CreateString("rgba(%u, %u, %u, %u)", c.red, c.green, c.blue, c.alpha);
		});

		constructor.Bind("search_players_query", &search_query);

		InitializePlayerEntryDataModel(constructor);

		constructor.RegisterArray<decltype(entries)>();
		constructor.Bind("players", &entries);

		data_model = constructor.GetModelHandle();
	}
}

void PlayerList::Filter()
{
	if (search_query.empty())
	{
		entries = data;
		return;
	}

	const auto search = [](const PlayerEntry& player, const Rml::String& query) -> bool {
		if (player.is_local)
			return true;

		Rml::String needle = Rml::StringUtilities::ToLower(player.name);
		if (needle.find(query) != Rml::String::npos)
			return true;

		needle = Rml::StringUtilities::ToLower(player.tag_name);
		if (needle.find(query) != Rml::String::npos)
			return true;

		return false;
	};

	// Dump all the current data first.
	entries.clear();

	// Use lowercase strings for querying.
	const Rml::String query = Rml::StringUtilities::ToLower(search_query);

	for (const PlayerEntry& player : data)
		if (search(player, query))
			entries.emplace_back(player);
}

void PlayerList::Sort()
{
	auto start_it = std::partition(entries.begin(), entries.end(), [](const PlayerEntry& player) { return player.is_local; });
	std::sort(start_it, entries.end(), [](const PlayerEntry& a, const PlayerEntry& b) { return a.score > b.score; });
}

void PlayerList::OnScrollUpdate()
{
	RMLUI_ZoneScoped;

	UpdateDisplayedRows();
}

void PlayerList::UpdateDisplayedRows()
{
	const float head_height = players_element->QuerySelector("thead")->GetClientHeight();
	// #todo The height should be fetched from the row itself or should be parsed from the styles.
	const float row_height = 22.f * document->GetContext()->GetDensityIndependentPixelRatio();

	const float scroll_offset = Rml::Math::Max(players_element->GetScrollTop() - head_height, 0.f);
	const float height = players_element->GetClientHeight();

	data_range.x = Rml::Math::RoundDownToInteger(scroll_offset / row_height);
	data_range.y = Rml::Math::RoundUpToInteger((scroll_offset + height) / row_height) + 1;

	Filter();
	Sort();

	int entries_count = (int)entries.size();

	if (data_range.y > entries_count)
		data_range.y = entries_count;

	entries = {entries.begin() + data_range.x, entries.begin() + data_range.y};

	top_filler_element->SetProperty(Rml::PropertyId::Height, Rml::Property(data_range.x * row_height, Rml::Unit::PX));
	bottom_filler_element->SetProperty(Rml::PropertyId::Height, Rml::Property((entries_count - data_range.y) * row_height, Rml::Unit::PX));

	// #todo Dirty the variable only if the data must be updated or if the data range changes.
	RMLUI_ASSERT(data_model);
	data_model.DirtyVariable("players");
}

PlayerEntry GenerateFakePlayerEntry(bool is_local)
{
	PlayerEntry data{};
	data.is_local = is_local;
	data.entity_id = (uint32_t)Rml::Math::RandomInteger(100'000);
	data.score = (uint32_t)Rml::Math::RandomInteger(100);
	data.latency = (uint16_t)Rml::Math::RandomInteger(500);
	data.is_muted = Rml::Math::RandomBool();
	data.is_friend = Rml::Math::RandomBool();
	Rml::FormatString(data.tag_name, "tag %d", Rml::Math::RandomInteger(1'000'000));
	data.name = is_local ? "local player" : Rml::CreateString("player %d", data.entity_id);

	const auto random_color_component = []() -> uint8_t { return (uint8_t)Rml::Math::RandomInteger(0xff); };
	data.tag_color = Rml::Colourb(random_color_component(), random_color_component(), random_color_component());

	return data;
}
