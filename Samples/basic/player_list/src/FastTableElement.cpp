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

#include "FastTableElement.h"
#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/Profiling.h>

FastTableElement::FastTableElement(const Rml::String& tag) :
	Element(tag), body(nullptr), last_height(0), last_scrollable_height(0), last_scroll_offset(0), recalculate_elements_order(false)
{}

void FastTableElement::OnChildAdd(Rml::Element* child)
{
	if (child->GetTagName() == "tbody")
		body = child;
	else if (child->IsClassSet("filler_top"))
		top_filler.element = child;
	else if (child->IsClassSet("filler_bottom"))
		bottom_filler.element = child;
	else
		recalculate_elements_order = true;
}

void FastTableElement::OnChildRemove(Rml::Element* child)
{
	if (child->GetTagName() == "tbody")
		body = child;
	else if (child->IsClassSet("filler_top"))
		top_filler.element = child;
	else if (child->IsClassSet("filler_bottom"))
		bottom_filler.element = child;
	else
	{
		top_filler.RemoveHiddenElement(child);
		bottom_filler.RemoveHiddenElement(child);
		recalculate_elements_order = true;
	}
}

void FastTableElement::OnUpdate()
{
	if (Rml::Element* scrollable_container = GetClosestScrollableContainer())
	{
		float scrollable_height = scrollable_container->GetClientHeight();
		float scroll_offset = scrollable_container->GetScrollTop();
		float height = GetClientHeight();

		// Skip updating the view if the inner content size is relatively the same, the view area size is the same, or the scroll offset did not change.
		if (Rml::Math::Absolute(last_height - height) < 2.f && scrollable_height == last_scrollable_height && scroll_offset == last_scroll_offset)
			return;

		UpdateView(scrollable_height, scroll_offset);

		last_height = height;
		last_scrollable_height = scrollable_height;
		last_scroll_offset = scroll_offset;
	}
}

void FastTableElement::UpdateView(float scrollable_height, float scrollable_offset)
{
	RMLUI_ZoneScopedC(0xB22222);

	if (recalculate_elements_order)
	{
		RMLUI_ZoneScopedN("FastTableElement::RecalculateElementsOrder");
		top_filler.RecalculateElementsOrder();
		bottom_filler.RecalculateElementsOrder();
		recalculate_elements_order = false;
	}

	bool sort_elements = false;

	for (int i = 0; i < body->GetNumChildren(); ++i)
	{
		Rml::Element* child = body->GetChild(i);

		// Avoid touching the filler elements.
		if (child == top_filler.element || child == bottom_filler.element)
			continue;

		// Skip checking already hidden elements.
		if (child->GetComputedValues().display() == Rml::Style::Display::None)
			continue;

		float child_offset = child->GetOffsetTop();
		float child_height = child->GetOffsetHeight();

		// Layouting has not finished for this child.
		if (child_height == 0)
			continue;

		if (scrollable_offset > child_offset + child_height)
		{
			child->SetProperty(Rml::PropertyId::Display, Rml::Style::Display::None);
			top_filler.AddHiddenElement(child);
			sort_elements = true;
		}
		else if (child_offset > scrollable_height + scrollable_offset)
		{
			child->SetProperty(Rml::PropertyId::Display, Rml::Style::Display::None);
			bottom_filler.AddHiddenElement(child);
			sort_elements = true;
		}
	}

	if (sort_elements)
	{
		top_filler.SortElements();
		bottom_filler.SortElements();
	}

	const float child_height = CalculateApproximateChildHeight();
	float top_filler_height = top_filler.element->GetClientHeight();

	while (top_filler_height > scrollable_offset - 5.f)
	{
		if (!top_filler.HasHiddenElements())
			break;

		Rml::Element* child = top_filler.GetLastHiddenElement();
		child->SetProperty(Rml::PropertyId::Display, Rml::Style::Display::TableRow);

		top_filler.RemoveHiddenElement(child);
		top_filler_height -= child_height;
	}

	const float element_height = GetClientHeight();
	float bottom_filler_height = bottom_filler.element->GetClientHeight();

	while (bottom_filler_height > 0 && scrollable_height + scrollable_offset > element_height - bottom_filler_height)
	{
		if (!bottom_filler.HasHiddenElements())
			break;

		Rml::Element* child = bottom_filler.GetFirstHiddenElement();
		child->SetProperty(Rml::PropertyId::Display, Rml::Style::Display::TableRow);

		bottom_filler.RemoveHiddenElement(child);
		bottom_filler_height -= child_height;
	}

	top_filler.UpdateHeight(child_height);
	bottom_filler.UpdateHeight(child_height);
}

float FastTableElement::CalculateApproximateChildHeight()
{
	return GetClientHeight() / (body->GetNumChildren() - 2);
}

////////////////////////////////////////////////////////////////////////////////

FastTableFiller::FastTableFiller() : element(nullptr)
{
	hidden_elements.reserve(0x200);
}

static const Rml::String ELEMENT_ATTR_DATA_ORDER{"data-order"};

void FastTableFiller::AddHiddenElement(Rml::Element* element)
{
	int order = element->GetAttribute(ELEMENT_ATTR_DATA_ORDER)->Get<int>();
	hidden_elements.push_back({element, order});
}

template <typename T, typename PRED>
static void EraseIf(T& container, PRED predicate)
{
	container.erase(std::remove_if(container.begin(), container.end(), predicate), container.end());
}

void FastTableFiller::RemoveHiddenElement(Rml::Element* element)
{
	EraseIf(hidden_elements, [element](const HiddenElement& it) { return it.element == element; });
}

void FastTableFiller::RecalculateElementsOrder()
{
	for (HiddenElement& it : hidden_elements)
		it.order = it.element->GetAttribute(ELEMENT_ATTR_DATA_ORDER)->Get<int>();
}

void FastTableFiller::SortElements()
{
	std::sort(hidden_elements.begin(), hidden_elements.end(), [](const HiddenElement& a, const HiddenElement& b) { return a.order < b.order; });
}

bool FastTableFiller::HasHiddenElements() const
{
	return !hidden_elements.empty();
}

Rml::Element* FastTableFiller::GetFirstHiddenElement() const
{
	RMLUI_ASSERT(HasHiddenElements());
	return hidden_elements.front().element;
}

Rml::Element* FastTableFiller::GetLastHiddenElement() const
{
	RMLUI_ASSERT(HasHiddenElements());
	return hidden_elements.back().element;
}

void FastTableFiller::UpdateHeight(float approximate_child_height)
{
	float height = hidden_elements.size() * approximate_child_height;
	element->SetProperty(Rml::PropertyId::Height, Rml::Property(height, Rml::Unit::PX));
}
