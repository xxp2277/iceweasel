/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/WindowGlobalActor.h"

#include "nsContentUtils.h"
#include "mozJSComponentLoader.h"
#include "mozilla/ContentBlockingAllowList.h"
#include "mozilla/Logging.h"
#include "mozilla/dom/JSActorService.h"
#include "mozilla/dom/JSWindowActorParent.h"
#include "mozilla/dom/JSWindowActorChild.h"

namespace mozilla {
namespace dom {

WindowGlobalInit WindowGlobalActor::AboutBlankInitializer(
    dom::BrowsingContext* aBrowsingContext, nsIPrincipal* aPrincipal) {
  MOZ_ASSERT(aBrowsingContext);
  MOZ_ASSERT(aPrincipal);

  nsCOMPtr<nsIURI> documentURI;
  Unused << NS_NewURI(getter_AddRefs(documentURI), "about:blank");

  uint64_t outerWindowId = nsContentUtils::GenerateWindowId();
  uint64_t innerWindowId = nsContentUtils::GenerateWindowId();

  nsCOMPtr<nsIPrincipal> contentBlockingAllowListPrincipal;
  ContentBlockingAllowList::ComputePrincipal(
      aPrincipal, getter_AddRefs(contentBlockingAllowListPrincipal));

  return WindowGlobalInit(aPrincipal, contentBlockingAllowListPrincipal,
                          documentURI, aBrowsingContext, innerWindowId,
                          outerWindowId);
}

void WindowGlobalActor::ConstructActor(const nsACString& aName,
                                       JS::MutableHandleObject aActor,
                                       ErrorResult& aRv) {
  MOZ_ASSERT(nsContentUtils::IsSafeToRunScript());

  JSActor::Type actorType = GetSide();
  MOZ_ASSERT_IF(actorType == JSActor::Type::Parent, XRE_IsParentProcess());

  // Constructing an actor requires a running script, so push an AutoEntryScript
  // onto the stack.
  AutoEntryScript aes(xpc::PrivilegedJunkScope(),
                      "WindowGlobalActor construction");
  JSContext* cx = aes.cx();

  RefPtr<JSActorService> actorSvc = JSActorService::GetSingleton();
  if (!actorSvc) {
    aRv.ThrowNotSupportedError("Could not acquire actor service");
    return;
  }

  RefPtr<JSWindowActorProtocol> proto =
      actorSvc->GetJSWindowActorProtocol(aName);
  if (!proto) {
    aRv.ThrowNotSupportedError(nsPrintfCString(
        "Could not get JSWindowActorProtocol: %s is not registered",
        PromiseFlatCString(aName).get()));
    return;
  }

  if (!proto->Matches(BrowsingContext(), GetDocumentURI(), GetRemoteType())) {
    aRv.Throw(NS_ERROR_NOT_AVAILABLE);
    return;
  }

  // Load the module using mozJSComponentLoader.
  RefPtr<mozJSComponentLoader> loader = mozJSComponentLoader::Get();
  MOZ_ASSERT(loader);

  JS::RootedObject global(cx);
  JS::RootedObject exports(cx);

  const JSWindowActorProtocol::Sided* side;
  if (actorType == JSActor::Type::Parent) {
    side = &proto->Parent();
  } else {
    side = &proto->Child();
  }

  // Support basic functionally such as SendAsyncMessage and SendQuery for
  // unspecified moduleURI.
  if (!side->mModuleURI) {
    RefPtr<JSActor> actor;
    if (actorType == JSActor::Type::Parent) {
      actor = new JSWindowActorParent();
    } else {
      actor = new JSWindowActorChild();
    }

    JS::Rooted<JS::Value> wrapper(cx);
    if (!ToJSValue(cx, actor, &wrapper)) {
      aRv.NoteJSContextException(cx);
      return;
    }

    MOZ_ASSERT(wrapper.isObject());
    aActor.set(&wrapper.toObject());
    return;
  }

  aRv = loader->Import(cx, side->mModuleURI.ref(), &global, &exports);
  if (aRv.Failed()) {
    return;
  }

  MOZ_ASSERT(exports, "null exports!");

  // Load the specific property from our module.
  JS::RootedValue ctor(cx);
  nsAutoCString ctorName(aName);
  ctorName.Append(actorType == JSActor::Type::Parent
                      ? NS_LITERAL_CSTRING("Parent")
                      : NS_LITERAL_CSTRING("Child"));
  if (!JS_GetProperty(cx, exports, ctorName.get(), &ctor)) {
    aRv.NoteJSContextException(cx);
    return;
  }

  if (NS_WARN_IF(!ctor.isObject())) {
    nsPrintfCString message("Could not find actor constructor '%s'",
                            ctorName.get());
    aRv.ThrowNotFoundError(message);
    return;
  }

  // Invoke the constructor loaded from the module.
  if (!JS::Construct(cx, ctor, JS::HandleValueArray::empty(), aActor)) {
    aRv.NoteJSContextException(cx);
    return;
  }
}

}  // namespace dom
}  // namespace mozilla
