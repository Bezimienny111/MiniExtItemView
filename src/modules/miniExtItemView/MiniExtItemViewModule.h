#pragma once

#include "common/TextWrap.h"
#include <filesystem>
#include <fstream>

namespace GOTHIC_NAMESPACE::MiniExtItemView
{
	namespace Feedback
	{
		inline void Trace(const char* event, zCView* view = nullptr, const zSTRING* text = nullptr,
			int left = 0, int top = 0, const void* item = nullptr, const void* renderTarget = nullptr)
		{
			static int entries = 0;
			static bool started = false;
			if (std::strcmp(event, "DrawItemInfo begin") == 0)
				started = true;
			if (!started || entries >= 250)
				return;
			char executable[MAX_PATH];
			const auto length = GetModuleFileNameA(nullptr, executable, MAX_PATH);
			if (!length || length >= MAX_PATH)
				return;
			std::ofstream output(std::filesystem::path(executable).parent_path() / "MiniExtItemView-layer.log",
				entries == 0 ? std::ios::trunc : std::ios::app);
			if (!output)
				return;
			if (entries == 0)
				output << "MiniExtItemView layer-trace-20261007-1\n";
			output << entries++ << " engine=" << ENGINE << " " << event;
			if (item || renderTarget)
				output << " item=" << item << " target=" << renderTarget;
			if (view)
				output << " view=" << view << " owner=" << view->owner
					<< " pixel=" << view->pposx << "," << view->pposy << "," << view->psizex << "," << view->psizey;
			if (text)
				output << " text=" << text->ToChar() << " at=" << left << "," << top;
			output << '\n';
		}
	}

	struct NativeText
	{
		int left;
		int top;
		std::string text;
		zCOLOR color;
	};

	struct DrawPass
	{
		struct Glyph
		{
			NativeText text;
			zCFont* font;
		};
		oCItemContainer* container;
		DrawPass* previous;
		std::vector<Glyph> glyphs;
		int left = 0, top = 0, width = 0, height = 0;
		bool replaying = false;
		static inline DrawPass* current = nullptr;
		explicit DrawPass(oCItemContainer* value) : container(value), previous(current) { current = this; }
		DrawPass(const DrawPass&) = delete;
		DrawPass& operator=(const DrawPass&) = delete;
		~DrawPass() { current = previous; }
		bool Capture(zCView* panel, int pixelLeft, int pixelTop, const zSTRING& text)
		{
			if (replaying || !container || panel != container->viewItemInfo || !panel->GetFont())
				return false;
			if (glyphs.empty() || left != panel->vposx || top != panel->vposy
				|| width != panel->vsizex || height != panel->vsizey)
			{
				glyphs.clear();
				left = panel->vposx; top = panel->vposy;
				width = panel->vsizex; height = panel->vsizey;
			}
			Glyph glyph{ { panel->anx(pixelLeft - panel->pposx), panel->any(pixelTop - panel->pposy),
				text.ToChar(), panel->fontColor }, panel->GetFont() };
			auto existing = std::find_if(glyphs.begin(), glyphs.end(), [&](const auto& stored)
				{ return stored.text.left == glyph.text.left && stored.text.top == glyph.text.top; });
			if (existing == glyphs.end())
				glyphs.push_back(std::move(glyph));
			else
				*existing = std::move(glyph);
			return true;
		}
	};

	struct DrawContext
	{
		oCItemContainer* container;
		DrawContext* previous;
		std::vector<NativeText> texts;
		bool drawn = false;
		zCView* preview = nullptr;
		int previewLeft = 0;
		static inline DrawContext* current = nullptr;
		explicit DrawContext(oCItemContainer* value) : container(value), previous(current) { current = this; }
		DrawContext(const DrawContext&) = delete;
		DrawContext& operator=(const DrawContext&) = delete;
		~DrawContext()
		{
			if (preview)
				preview->SetPos(previewLeft, preview->vposy);
			current = previous;
		}
	};

	inline bool CaptureText(zCView* panel, int left, int top, const zSTRING& text, float time, zCOLOR* color)
	{
		if constexpr (ENGINE == Engine_G1 || ENGINE == Engine_G1A)
		{
			auto* context = DrawContext::current;
			if (!context || !context->container || !panel
				|| panel != context->container->viewItemInfo || time != -2.0f)
				return false;
			if (context->drawn)
				return true;
			NativeText entry{ left, top, text.ToChar(), color ? *color : panel->fontColor };
			auto existing = std::find_if(context->texts.begin(), context->texts.end(),
				[&](const auto& stored) { return stored.left == left && stored.top == top; });
			if (existing == context->texts.end())
				context->texts.push_back(std::move(entry));
			else
				*existing = std::move(entry);
			return true;
		}
		else
			return false;
	}

	struct Field
	{
		std::string label;
		std::string value;
		zCOLOR labelColor;
		zCOLOR valueColor;
	};

	struct Row
	{
		std::string label;
		std::string value;
		int valueLeft = 0;
		zCOLOR labelColor;
		zCOLOR valueColor;
	};

	struct Layout
	{
		int height = 0;
		int textWidth = 0;
		int bodyTop = 0;
		int padding = 0;
		std::vector<std::string> header;
		std::vector<Row> rows;
	};

	inline Layout BuildLayout(zCFont* font, const std::string& title,
		const std::vector<Field>& fields, int width, int nativeHeight, int availableHeight, int panelWidth = 0)
	{
		Layout layout;
		if (!font || font->GetFontY() <= 0 || width < 80 || nativeHeight <= 0)
			return layout;
		const int lineHeight = font->GetFontY();
		const int maximum = (std::min)(nativeHeight + nativeHeight / 2, availableHeight);
		layout.padding = (std::max)(4, lineHeight / 2);
		layout.textWidth = width - layout.padding * 2;
		const int valueRight = (std::max)(width, panelWidth) - layout.padding * 2;
		if (layout.textWidth < lineHeight * 3 || maximum < lineHeight * 4 + layout.padding * 2)
			return {};
		layout.header = CZMCommon::WrapText(font, title, valueRight, 2);
		if (layout.header.empty())
			layout.header.emplace_back();
		layout.bodyTop = layout.padding + static_cast<int>(layout.header.size()) * lineHeight + lineHeight / 2;
		const int capacity = (maximum - layout.bodyTop - layout.padding) / lineHeight;
		int remainingFields = 0;
		for (const auto& field : fields)
			if (!field.label.empty() || !field.value.empty())
				++remainingFields;
		if (capacity < remainingFields)
			return {};
		for (const auto& field : fields)
		{
			if (field.label.empty() && field.value.empty())
				continue;
			--remainingFields;
			const int budget = capacity - static_cast<int>(layout.rows.size()) - remainingFields;
			const int valueWidth = font->GetFontX(zSTRING(field.value.c_str()));
			const int labelWidth = layout.textWidth - (panelWidth > width || field.value.empty() ? 0 : valueWidth + 12);
			if (labelWidth < lineHeight || valueWidth > valueRight)
				return {};
			auto lines = CZMCommon::WrapText(font, field.label, labelWidth, budget);
			if (lines.empty())
				lines.emplace_back();
			bool first = true;
			for (auto& line : lines)
			{
				layout.rows.push_back(Row{ std::move(line), first ? field.value : std::string(),
					valueRight - valueWidth, field.labelColor, field.valueColor });
				first = false;
			}
		}
		const int needed = layout.bodyTop + static_cast<int>(layout.rows.size()) * lineHeight + layout.padding;
		layout.height = std::clamp(needed, (std::min)(nativeHeight, maximum), maximum);
		return layout;
	}

	inline void ClearText(zCView& panel)
	{
		panel.ClrPrintwin();
		if constexpr (ENGINE == Engine_G1 || ENGINE == Engine_G1A)
			panel.textLines.DeleteListDatas();
	}

	class PanelScope
	{
		zCView& panel;
		zCView* preview;
		int left, top, width, height;
		int previewLeft = 0, previewTop = 0, previewWidth = 0, previewHeight = 0;
		zCFont* font;
		zCOLOR fontColor;
		int maxTextLength;
		zSTRING textMaxLength;
		zCView* previousTextPanel;
	public:
		static inline zCView* textPanel = nullptr;
		PanelScope(zCView& view, zCView* model)
			: panel(view), preview(model), left(view.vposx), top(view.vposy), width(view.vsizex), height(view.vsizey),
			font(view.font), fontColor(view.fontColor), maxTextLength(view.maxTextLength), textMaxLength(view.textMaxLength),
			previousTextPanel(textPanel)
		{
			textPanel = &view;
			if (preview)
			{
				previewLeft = preview->vposx;
				previewTop = preview->vposy;
				previewWidth = preview->vsizex;
				previewHeight = preview->vsizey;
			}
		}
		PanelScope(const PanelScope&) = delete;
		PanelScope& operator=(const PanelScope&) = delete;
		~PanelScope()
		{
			textPanel = previousTextPanel;
			ClearText(panel);
			panel.SetSize(width, height);
			panel.SetPos(left, top);
			if (preview)
			{
				preview->SetSize(previewWidth, previewHeight);
				preview->SetPos(previewLeft, previewTop);
			}
			panel.font = font;
			panel.fontColor = fontColor;
			panel.maxTextLength = maxTextLength;
			panel.textMaxLength = textMaxLength;
		}
	};

	template<class Blit>
	void DrawFinalText(DrawPass& pass, Blit blit)
	{
		auto* panel = pass.container ? pass.container->viewItemInfo : nullptr;
		if (pass.glyphs.empty() || !screen || !panel || (panel->owner && panel->owner != screen))
			return;
		struct AttachmentScope
		{
			zCView* desktop;
			zCView* panel;
			bool attached;
			~AttachmentScope() { if (attached) desktop->RemoveItem(panel); }
		} attachment{ screen, panel, !panel->owner };
		if (attachment.attached)
			screen->InsertItem(panel);
		PanelScope geometry(*panel, nullptr);
		struct BackgroundScope
		{
			zCView* panel;
			zCTexture* background;
			DrawPass& pass;
			~BackgroundScope() { panel->backTex = background; pass.replaying = false; }
		} background{ panel, panel->backTex, pass };
		pass.replaying = true;
		panel->backTex = nullptr;
		ClearText(*panel);
		panel->SetSize(pass.width, pass.height);
		panel->SetPos(pass.left, pass.top);
		for (auto& glyph : pass.glyphs)
		{
			panel->font = glyph.font;
			panel->PrintTimed(glyph.text.left, glyph.text.top, zSTRING(glyph.text.text.c_str()), -2.0f, &glyph.text.color);
		}
		Feedback::Trace("Final text begin", panel);
		blit(panel);
		Feedback::Trace("Final text end", panel);
	}

	template<class Blit>
	bool Draw(zCView* panel, zCView* preview, Blit blit, const std::vector<NativeText>* captured = nullptr)
	{
		if (!screen || !panel || panel->owner != screen || !panel->GetFont())
			return false;
		std::vector<NativeText> texts;
		if (captured)
			texts = *captured;
		else for (auto* node = panel->textLines.next; node; node = node->next)
		{
			if (!node->data)
				continue;
			if (texts.size() == 13)
				return false;
			auto& text = *node->data;
			texts.push_back({ text.posx, text.posy, text.text.ToChar(), text.colored ? text.color : panel->fontColor });
		}
		if (texts.empty())
			return false;
		std::sort(texts.begin(), texts.end(), [](const auto& first, const auto& second)
			{ return first.top != second.top ? first.top < second.top : first.left < second.left; });
		auto title = texts.front();
		std::vector<Field> fields;
		int lastTop = title.top;
		for (std::size_t index = 1; index < texts.size(); ++index)
		{
			const auto& text = texts[index];
			if (text.top == title.top)
				return false;
			if (text.top != lastTop)
			{
				if (fields.size() == 6)
					return false;
				fields.push_back({ text.text, {}, text.color, text.color });
				lastTop = text.top;
			}
			else
			{
				if (!fields.back().value.empty())
					return false;
				fields.back().value = text.text;
				fields.back().valueColor = text.color;
			}
		}
		if (!preview || preview->owner != panel)
			preview = nullptr;
		const int bottom = panel->vposy + panel->vsizey;
		int width = panel->psizex;
		const int modelLeft = preview ? panel->nax(preview->vposx) : 0;
		const int modelTop = preview ? panel->nay(preview->vposy) : 0;
		const int modelWidth = preview ? preview->psizex : 0;
		const int modelHeight = preview ? preview->psizey : 0;
		if (preview)
			width = (std::min)(width, modelLeft);
		auto layout = BuildLayout(panel->GetFont(), title.text, fields, width, panel->psizey,
			screen->nay(bottom) - 12, panel->psizex);
		if (layout.height <= 0)
			return false;
		const int nativeTop = panel->vposy;
		const int height = (std::min)(panel->vsizey + panel->vsizey / 2, screen->any(layout.height));
		PanelScope scope(*panel, preview);
		ClearText(*panel);
		panel->SetSize(panel->vsizex, height);
		panel->SetPos(panel->vposx, bottom - height);
		if (preview)
		{
			preview->SetSize(panel->anx(modelWidth), panel->any(modelHeight));
			preview->SetPos(panel->anx(modelLeft), panel->any(modelTop + screen->nay(nativeTop - panel->vposy)));
		}
		int top = layout.padding;
		for (const auto& line : layout.header)
		{
			panel->PrintTimedCX(panel->any(top), zSTRING(line.c_str()), -2.0f, &title.color);
			top += panel->GetFont()->GetFontY();
		}
		top = layout.bodyTop;
		for (auto& row : layout.rows)
		{
			panel->PrintTimed(panel->anx(layout.padding), panel->any(top), zSTRING(row.label.c_str()), -2.0f, &row.labelColor);
			if (!row.value.empty())
				panel->PrintTimed(panel->anx(layout.padding + row.valueLeft), panel->any(top), zSTRING(row.value.c_str()), -2.0f, &row.valueColor);
			top += panel->GetFont()->GetFontY();
		}
		blit();
		return true;
	}
}