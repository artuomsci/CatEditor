#ifndef COMMON_H
#define COMMON_H

#include <qnamespace.h>
#include <QStringList>

// Node data
constexpr int eID = Qt::UserRole + 0;

//----------------------------------------------------------------------
static const double  blob_radius       = 20.0;
static const int     blob_layer        = 2;

static const int     label_layer       = 1;

static const double  arrow_head_size   = blob_radius * 1.0;

static const int     scene_size        = 10000;

static const QStringList cat_types = {{"Double"},{"Float"},{"Int"},{"String"}};

#endif
