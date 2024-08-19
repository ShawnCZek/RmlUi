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

#ifndef RMLUI_SAMPLES_PLAYERLIST_FASTTABLEELEMENT_H
#define RMLUI_SAMPLES_PLAYERLIST_FASTTABLEELEMENT_H

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Types.h>

class FastTableFiller {
public:
	FastTableFiller();

	void AddHiddenElement(Rml::Element* element);
	void RemoveHiddenElement(Rml::Element* element);

	void RecalculateElementsOrder();
	void SortElements();

	bool HasHiddenElements() const;
	Rml::Element* GetFirstHiddenElement() const;
	Rml::Element* GetLastHiddenElement() const;

	void UpdateHeight(float approximate_child_height);

private:
	struct HiddenElement {
		// The element that was hidden.
		Rml::Element* element;
		// The numerical position of the element in the entire list.
		// #todo This should be replaced by the element offset or any other attribute to avoid
		// relying on the data model. However, I could not have thought of any other reliable
		// way to sort elements, especially as they can be removed/added during runtime.
		int order;
	};

	// Elements that are hidden and are replaced by the filler element.
	Rml::Vector<HiddenElement> hidden_elements;

public:
	// The filler element replacing the hidden elements.
	Rml::Element* element;
};

class FastTableElement final : public Rml::Element {
public:
	FastTableElement(const Rml::String& tag);

private:
	void OnChildAdd(Rml::Element* child) override;
	void OnChildRemove(Rml::Element* child) override;

	void OnUpdate() override;

private:
	void UpdateView(float scrollable_height, float scrollable_offset);

	float CalculateApproximateChildHeight();

private:
	Rml::Element* body;

	// The current element's height during the last update.
	float last_height;
	// The height of the closest scrollable container during the last update.
	float last_scrollable_height;
	// The scroll offset of the closest scrollable container during the last update.
	float last_scroll_offset;

	FastTableFiller top_filler;
	FastTableFiller bottom_filler;

	// A flag to mark whether elements' order must be recalculated during the next update.
	bool recalculate_elements_order;
};

#endif
