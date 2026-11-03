#include "modules/miniExtItemView/MiniExtItemViewModule.h"

namespace GOTHIC_NAMESPACE::MiniExtItemView
{
	void __fastcall View_PrintTimed(zCView* self, void* vtable, int left, int top,
		const zSTRING& text, float time, zCOLOR* color);
	auto Hook_View_PrintTimed = Union::CreateHook(SIGNATURE_OF(&zCView::PrintTimed), &View_PrintTimed, Union::HookType::Hook_Detours);
	void __fastcall View_PrintTimed(zCView* self, void* vtable, int left, int top,
		const zSTRING& text, float time, zCOLOR* color)
	{
		if (!CaptureText(self, left, top, text, time, color))
			Hook_View_PrintTimed(self, vtable, left, top, text, time, color);
	}

	void __fastcall View_PrintTimedCX(zCView* self, void* vtable, int top,
		const zSTRING& text, float time, zCOLOR* color);
	auto Hook_View_PrintTimedCX = Union::CreateHook(SIGNATURE_OF(&zCView::PrintTimedCX), &View_PrintTimedCX, Union::HookType::Hook_Detours);
	void __fastcall View_PrintTimedCX(zCView* self, void* vtable, int top,
		const zSTRING& text, float time, zCOLOR* color)
	{
		if constexpr (ENGINE == Engine_G1 || ENGINE == Engine_G1A)
		{
			if (self->GetFont() && CaptureText(self,
				(8192 - self->anx(self->GetFont()->GetFontX(text))) / 2, top, text, time, color))
				return;
		}
		Hook_View_PrintTimedCX(self, vtable, top, text, time, color);
	}

	void __fastcall View_PrintChars(zCView* self, void* vtable, int left, int top, const zSTRING& text);
	auto Hook_View_PrintChars = Union::CreateHook(SIGNATURE_OF(&zCView::PrintChars), &View_PrintChars, Union::HookType::Hook_Detours);
	void __fastcall View_PrintChars(zCView* self, void* vtable, int left, int top, const zSTRING& text)
	{
		if (self == PanelScope::textPanel)
		{
			if (DrawPass::current && DrawPass::current->Capture(self, left, top, text))
			{
				Feedback::Trace("Queued Mini PrintChars", self, &text, left, top);
				return;
			}
			Feedback::Trace("Mini PrintChars", self, &text, left, top);
		}
		Hook_View_PrintChars(self, vtable, left, top, text);
	}

	void __fastcall ItemContainer_Draw(oCItemContainer* self, void* vtable);
	auto Hook_ItemContainer_Draw = Union::CreateHook(SIGNATURE_OF(&oCItemContainer::Draw), &ItemContainer_Draw, Union::HookType::Hook_Detours);

	void __fastcall Item_RenderItem(oCItem* self, void* vtable, zCWorld* world, zCViewBase* view, float selected);
	auto Hook_Item_RenderItem = Union::CreateHook(
		SIGNATURE_OF(static_cast<void (oCItem::*)(zCWorld*, zCViewBase*, float)>(&oCItem::RenderItem)),
		&Item_RenderItem, Union::HookType::Hook_Detours);
	void __fastcall Item_RenderItem(oCItem* self, void* vtable, zCWorld* world, zCViewBase* view, float selected)
	{
		auto* context = DrawContext::current;
		auto* container = context ? context->container : nullptr;
		auto* preview = container ? container->viewItemInfoItem : nullptr;
		const bool infoPreview = preview && view == preview && container->viewItemInfo
			&& preview->owner == container->viewItemInfo;
		if (infoPreview)
		{
			Feedback::Trace("Info RenderItem begin", preview, nullptr, 0, 0, self, view);
			if (!context->preview)
			{
				context->preview = preview;
				context->previewLeft = preview->vposx;
			}
			preview->SetPos(5500 - preview->vsizex / 2, preview->vposy);
		}
		if (!infoPreview)
			Feedback::Trace("Other RenderItem begin", nullptr, nullptr, 0, 0, self, view);
		Hook_Item_RenderItem(self, vtable, world, view, selected);
		Feedback::Trace(infoPreview ? "Info RenderItem end" : "Other RenderItem end", nullptr, nullptr, 0, 0, self, view);
		if (infoPreview && zrenderer)
		{
			[]<class Renderer>(Renderer* renderer)
			{
				if constexpr (ENGINE == Engine_G2 || ENGINE == Engine_G2A)
					renderer->FlushAlphaPolys();
			}(zrenderer);
			zrenderer->FlushPolys();
		}
	}

	void __fastcall ItemContainer_DrawItemInfo(oCItemContainer* self, void* vtable, oCItem* item, zCWorld* world);
	auto Hook_ItemContainer_DrawItemInfo = Union::CreateHook(
		SIGNATURE_OF(&oCItemContainer::DrawItemInfo), &ItemContainer_DrawItemInfo, Union::HookType::Hook_Detours);
	void __fastcall ItemContainer_DrawItemInfo(oCItemContainer* self, void* vtable, oCItem* item, zCWorld* world)
	{
		DrawContext scope(self);
		Feedback::Trace("DrawItemInfo begin", self->viewItemInfo);
		Hook_ItemContainer_DrawItemInfo(self, vtable, item, world);
		Feedback::Trace("DrawItemInfo end", self->viewItemInfo);
	}

	void __fastcall View_Blit(zCView* self, void* vtable);
	auto Hook_View_Blit = Union::CreateHook(SIGNATURE_OF(&zCView::Blit), &View_Blit, Union::HookType::Hook_Detours);
	void __fastcall View_Blit(zCView* self, void* vtable)
	{
		auto* context = DrawContext::current;
		auto* container = context ? context->container : nullptr;
		DrawContext scope(nullptr);
		if (!container || self != container->viewItemInfo)
		{
			Hook_View_Blit(self, vtable);
			return;
		}
		Feedback::Trace("Info Blit", self);
		if constexpr (ENGINE == Engine_G1 || ENGINE == Engine_G1A)
		{
			if (context->drawn)
				return;
			if (context->texts.empty())
			{
				Hook_View_Blit(self, vtable);
				return;
			}
			context->drawn = true;
			if (!Draw(self, container->viewItemInfoItem, [&] { Hook_View_Blit(self, vtable); }, &context->texts))
			{
				ClearText(*self);
				for (auto& text : context->texts)
					Hook_View_PrintTimed(self, nullptr, text.left, text.top, zSTRING(text.text.c_str()), -2.0f, &text.color);
				Hook_View_Blit(self, vtable);
				ClearText(*self);
			}
		}
		else if (!Draw(self, container->viewItemInfoItem, [&] { Hook_View_Blit(self, vtable); }))
			Hook_View_Blit(self, vtable);
	}

	void __fastcall ItemContainer_Draw(oCItemContainer* self, void* vtable)
	{
		DrawPass pass(self);
		Hook_ItemContainer_Draw(self, vtable);
		DrawContext disabled(nullptr);
		DrawFinalText(pass, [](zCView* panel) { Hook_View_Blit(panel, nullptr); });
	}
}