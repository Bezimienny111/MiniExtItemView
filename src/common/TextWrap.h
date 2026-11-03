#pragma once

#include <ZenGin/zGothicAPI.h>

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace GOTHIC_NAMESPACE::CZMCommon
{
	inline std::vector<std::string> WrapText(zCFont* font, std::string_view text,
		int widthPx, int maxLines, int continuationIndentPx = 0)
	{
		std::vector<std::string> lines;
		if (!font || maxLines <= 0)
			return lines;
		lines.reserve(maxLines);
		const int letterDistance = font->GetLetterDistance();
		const auto characterWidth = [&](char character) { return font->GetWidth(character) + letterDistance; };

		size_t position = 0;
		for (int lineIndex = 0; lineIndex < maxLines; ++lineIndex)
		{
			while (position < text.size() && text[position] == ' ')
				++position;
			const int limit = (std::max)(0, widthPx - (lineIndex == 0 ? 0 : continuationIndentPx));
			std::string line;
			int usedWidth = 0;
			size_t breakLength = 0;
			size_t nextWord = std::string_view::npos;
			bool hardBreak = false;
			while (position < text.size())
			{
				if (text[position] == '\n')
				{
					++position;
					hardBreak = true;
					break;
				}
				if (text[position] == ' ')
				{
					while (position < text.size() && text[position] == ' ')
						++position;
					if (position == text.size())
						break;
					breakLength = line.size();
					nextWord = position;
					const int spaceWidth = characterWidth(' ');
					if (usedWidth + spaceWidth > limit)
						break;
					line += ' ';
					usedWidth += spaceWidth;
				}
				const int glyphWidth = characterWidth(text[position]);
				if (usedWidth + glyphWidth > limit)
					break;
				line += text[position++];
				usedWidth += glyphWidth;
			}
			if (position < text.size() && nextWord != std::string_view::npos && !hardBreak)
			{
				line.resize(breakLength);
				position = nextWord;
			}
			lines.push_back(std::move(line));
			if (position == text.size() || (lines.back().empty() && !hardBreak))
				break;
		}

		if (position < text.size())
		{
			const int lastWidth = (std::max)(0, widthPx - (lines.size() == 1 ? 0 : continuationIndentPx));
			std::string& last = lines.back();
			int usedWidth = font->GetFontX(zSTRING(last.c_str()));
			std::string ellipsis("...");
			int ellipsisWidth = font->GetFontX(zSTRING("..."));
			while (!last.empty() && (last.back() == ' ' || usedWidth + ellipsisWidth > lastWidth))
			{
				usedWidth -= characterWidth(last.back());
				last.pop_back();
			}
			while (!ellipsis.empty() && ellipsisWidth > lastWidth)
			{
				ellipsisWidth -= characterWidth(ellipsis.back());
				ellipsis.pop_back();
			}
			last += ellipsis;
		}
		return lines;
	}
}