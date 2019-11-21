/*
 * Copyright (c) 2018-2019 NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 *
 */

#ifndef _GST_LSRMSGCONV_H_
#define _GST_LSRMSGCONV_H_

#include <gst/base/gstbasetransform.h>
#include "lsrmsgconv.h"

G_BEGIN_DECLS

#define GST_TYPE_LSRMSGCONV   (gst_lsrmsgconv_get_type())
#define GST_LSRMSGCONV(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_LSRMSGCONV,GstLsrMsgConv))
#define GST_LSRMSGCONV_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_LSRMSGCONV,GstLsrMsgConvClass))
#define GST_IS_LSRMSGCONV(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_LSRMSGCONV))
#define GST_IS_LSRMSGCONV_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_LSRMSGCONV))

typedef struct _GstLsrMsgConv GstLsrMsgConv;
typedef struct _GstLsrMsgConvClass GstLsrMsgConvClass;

typedef LsrDsMsg2pCtx* (*lsrds_msg2p_ctx_create_ptr) (NvDsPayloadType type);

typedef void (*lsrds_msg2p_ctx_destroy_ptr) (LsrDsMsg2pCtx *ctx);

typedef NvDsPayload* (*lsrds_msg2p_generate_ptr) (LsrDsMsg2pCtx *ctx, NvDsFrameMeta *frame_meta, guint size);

typedef void (*lsrds_msg2p_release_ptr) (LsrDsMsg2pCtx *ctx, NvDsPayload *payload);

struct _GstLsrMsgConv
{
  GstBaseTransform parent;

  GQuark dsMetaQuark;
  gchar *configFile;
  gchar *msg2pLib;
  gpointer libHandle;
  gint compId;
  NvDsPayloadType paylodType;
  LsrDsMsg2pCtx *pCtx;

  lsrds_msg2p_ctx_create_ptr ctx_create;
  lsrds_msg2p_ctx_destroy_ptr ctx_destroy;
  lsrds_msg2p_generate_ptr msg2p_generate;
  lsrds_msg2p_release_ptr msg2p_release;
};

struct _GstLsrMsgConvClass
{
  GstBaseTransformClass parent_class;
};

GType gst_lsrmsgconv_get_type (void);

G_END_DECLS

#endif
