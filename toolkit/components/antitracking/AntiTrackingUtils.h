/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_antitrackingutils_h
#define mozilla_antitrackingutils_h

#include "mozilla/AlreadyAddRefed.h"
#include "nsStringFwd.h"

class nsPIDOMWindowInner;
class nsPIDOMWindowOuter;
class nsIChannel;
class nsIPermission;
class nsIPrincipal;
class nsIURI;

namespace mozilla {
namespace dom {
class BrowsingContext;
}  // namespace dom

class AntiTrackingUtils final {
 public:
  static already_AddRefed<nsPIDOMWindowInner> GetInnerWindow(
      dom::BrowsingContext* aBrowsingContext);

  static already_AddRefed<nsPIDOMWindowOuter> GetTopWindow(
      nsPIDOMWindowInner* aWindow);

  // Get the current document URI from a document channel as it is being loaded.
  static already_AddRefed<nsIURI> MaybeGetDocumentURIBeingLoaded(
      nsIChannel* aChannel);

  static void CreateStoragePermissionKey(const nsACString& aTrackingOrigin,
                                         nsACString& aPermissionKey);

  // Given a principal, returns the storage permission key that will be used for
  // the principal.  Returns true on success.
  static bool CreateStoragePermissionKey(nsIPrincipal* aPrincipal,
                                         nsACString& aKey);

  // Returns true if the permission passed in is a storage access permission
  // for the passed in principal argument.
  static bool IsStorageAccessPermission(nsIPermission* aPermission,
                                        nsIPrincipal* aPrincipal);

  // Returns true if the storage permission is granted for the given principal
  // and the storage permission key.
  static bool CheckStoragePermission(nsIPrincipal* aPrincipal,
                                     const nsAutoCString& aType,
                                     bool aIsInPrivateBrowsing,
                                     uint32_t* aRejectedReason,
                                     uint32_t aBlockedReason);

  // Returns true if the storage permission is granted for the given channel.
  // And this is meant to be called in the parent process.
  static bool HasStoragePermissionInParent(nsIChannel* aChannel);

  // Returns the toplevel inner window id, returns 0 if this is a toplevel
  // window.
  static uint64_t GetTopLevelAntiTrackingWindowId(
      dom::BrowsingContext* aBrowsingContext);

  // Returns the parent inner window id, returns 0 if this or the parent are not
  // a toplevel window. This is mainly used to determine the anti-tracking
  // storage area.
  static uint64_t GetTopLevelStorageAreaWindowId(
      dom::BrowsingContext* aBrowsingContext);

  // Returns the principal of the given browsing context.
  // This API should only be used either in child processes with an in-process
  // browsing context or in the parent process.
  static already_AddRefed<nsIPrincipal> GetPrincipal(
      dom::BrowsingContext* aBrowsingContext);

  // Returns the principal of the given browsing context and tracking origin.
  // This API should only be used either in child processes with an in-process
  // browsing context or in the parent process.
  static bool GetPrincipalAndTrackingOrigin(
      dom::BrowsingContext* aBrowsingContext, nsIPrincipal** aPrincipal,
      nsACString& aTrackingOrigin);

  // Retruns true if the given browsingContext is a first-level sub context,
  // i.e. a first-level iframe.
  static bool IsFirstLevelSubContext(dom::BrowsingContext* aBrowsingContext);

  // Retruns the cookie behavior of the given browsingContext,
  // return BEHAVIOR_REJECT when fail.
  static uint32_t GetCookieBehavior(dom::BrowsingContext* aBrowsingContext);
};

}  // namespace mozilla

#endif  // mozilla_antitrackingutils_h
