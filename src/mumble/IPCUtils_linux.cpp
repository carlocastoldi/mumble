// Copyright 2021-2022 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "IPCUtils.h"

#include <unistd.h>
#include <QtCore/QDir>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QString>

namespace Mumble {
	QDir getRuntimeDir(void) {
		QString xdgRuntimePath = QProcessEnvironment::systemEnvironment().value(QLatin1String("XDG_RUNTIME_DIR"));
		QString mumbleRuntimePath;
		if (!xdgRuntimePath.isNull()) {
		    mumbleRuntimePath = QDir(xdgRuntimePath).absolutePath() + QLatin1String("/mumble/");
		} else {
			mumbleRuntimePath = QLatin1String("/run/user/") + QString::number(getuid()) + QLatin1String("/mumble/");
		}
		QDir mumbleRuntimeDir = QDir(mumbleRuntimePath);
		mumbleRuntimeDir.mkpath(".");
		return mumbleRuntimeDir;
	}

}; // namespace Mumble
