// Copyright 2021-2022 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "IPCUtils.h"

#include <QtCore/QDir>
#include <QtCore/QString>

namespace Mumble {
	QDir getRuntimeDir(void) {
		QDir mumbleRuntimeDir = QDir::home();
		return mumbleRuntimeDir;
	}

}; // namespace Mumble
