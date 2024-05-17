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

#ifndef RMLUI_CORE_FONTMATCH_H
#define RMLUI_CORE_FONTMATCH_H

#include "StringUtilities.h"
#include "StyleTypes.h"
#include "Types.h"

namespace Rml {

/**
    Specifications to construct a font family, similarly to the @font-face CSS rule.
    Deviations have been made where necessary. For more details, discuss the documentation of member variables.
 */

struct FontMatch {
	// Name of an existing font family.
	StringView name;
	// A collection of Unicode ranges that the font face may support. Leave empty to use the font's character map.
	Span<Pair<char32_t, char32_t>> character_ranges;
	// A range of font weights to load for the font family.
	Pair<Style::FontWeight, Style::FontWeight> font_weight;

	FontMatch() : font_weight({Style::FontWeight::Auto, Style::FontWeight::Auto}) {}
};

} // namespace Rml
#endif
