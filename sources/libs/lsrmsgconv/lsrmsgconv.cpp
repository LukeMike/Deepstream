/*
 * Copyright (c) 2018, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 *
 */

#include "lsrmsgconv.h"
#include <json-glib/json-glib.h>
#include <uuid.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>
#include <unordered_map>

#include<ctime>

using namespace std;


#define CONFIG_GROUP_SENSOR "sensor"
#define CONFIG_GROUP_PLACE "place"
#define CONFIG_GROUP_ANALYTICS "analytics"

#define CONFIG_KEY_COORDINATE "coordinate"
#define CONFIG_KEY_DESCRIPTION "description"
#define CONFIG_KEY_ENABLE  "enable"
#define CONFIG_KEY_ID "id"
#define CONFIG_KEY_LANE "lane"
#define CONFIG_KEY_LEVEL "level"
#define CONFIG_KEY_LOCATION "location"
#define CONFIG_KEY_NAME "name"
#define CONFIG_KEY_SOURCE "source"
#define CONFIG_KEY_TYPE "type"
#define CONFIG_KEY_VERSION "version"


#define CONFIG_KEY_PLACE_SUB_FIELD1 "place-sub-field1"
#define CONFIG_KEY_PLACE_SUB_FIELD2 "place-sub-field2"
#define CONFIG_KEY_PLACE_SUB_FIELD3 "place-sub-field3"

#define DEFAULT_CSV_FIELDS 10


#define CHECK_ERROR(error) \
    if (error) { \
      cout << "Error: " << error->message << endl; \
      goto done; \
    }

static JsonObject*
generate_nvosdcolorparams (LsrDsMsg2pCtx *ctx, NvOSD_ColorParams color_params)
{
  JsonObject *NvOSD_ColorParamsObj;

    NvOSD_ColorParamsObj = json_object_new ();

  json_object_set_double_member (NvOSD_ColorParamsObj, "red", color_params.red);
  json_object_set_double_member (NvOSD_ColorParamsObj, "green", color_params.green);
  json_object_set_double_member (NvOSD_ColorParamsObj, "blue", color_params.blue);
  json_object_set_double_member (NvOSD_ColorParamsObj, "alpha", color_params.alpha);

  return NvOSD_ColorParamsObj;
}

static JsonObject*
generate_nvosdrectparams (LsrDsMsg2pCtx *ctx, NvOSD_RectParams rect_params)
{
  JsonObject *NvOSD_RectParamsObj;
  JsonObject *NvOSD_ColorParamsObj;

  NvOSD_RectParamsObj = json_object_new ();

  json_object_set_int_member (NvOSD_RectParamsObj, "left", rect_params.left);
  json_object_set_int_member (NvOSD_RectParamsObj, "top", rect_params.top);
  json_object_set_int_member (NvOSD_RectParamsObj, "width", rect_params.width);
  json_object_set_int_member (NvOSD_RectParamsObj, "height", rect_params.height);
  if (ctx->payloadType == NVDS_PAYLOAD_DEEPSTREAM) {
    json_object_set_int_member (NvOSD_RectParamsObj, "border_width", rect_params.border_width);
    json_object_set_int_member (NvOSD_RectParamsObj, "has_bg_color", rect_params.has_bg_color);
    json_object_set_int_member (NvOSD_RectParamsObj, "has_color_info", rect_params.has_color_info);
    json_object_set_int_member (NvOSD_RectParamsObj, "color_id", rect_params.color_id);
  }

  if (ctx->payloadType == NVDS_PAYLOAD_DEEPSTREAM) {
    /******************** border_color */
    NvOSD_ColorParamsObj = generate_nvosdcolorparams (ctx, rect_params.border_color);
    json_object_set_object_member (NvOSD_RectParamsObj, "border_color", NvOSD_ColorParamsObj);
    /******************** border_color */
  }

  if (ctx->payloadType == NVDS_PAYLOAD_DEEPSTREAM) {
    /******************** bg_color */
    NvOSD_ColorParamsObj = generate_nvosdcolorparams (ctx, rect_params.bg_color);
    json_object_set_object_member (NvOSD_RectParamsObj, "bg_color", NvOSD_ColorParamsObj);
    /******************** bg_color */
  }

  return NvOSD_RectParamsObj;
}

static JsonObject*
generate_nvosdfontparams (LsrDsMsg2pCtx *ctx, NvOSD_FontParams font_params)
{
  JsonObject *NvOSD_ColorParamsObj;
  JsonObject *NvOSD_FontParamsObj;

    NvOSD_FontParamsObj = json_object_new ();

  json_object_set_string_member (NvOSD_FontParamsObj, "font_name", font_params.font_name);
  json_object_set_int_member (NvOSD_FontParamsObj, "font_size", font_params.font_size);

  /****************************** font_color */
  NvOSD_ColorParamsObj = generate_nvosdcolorparams (ctx, font_params.font_color);
  json_object_set_object_member (NvOSD_FontParamsObj, "font_color", NvOSD_ColorParamsObj);
  /****************************** font_color */

  return NvOSD_FontParamsObj;
}

static JsonObject*
generate_nvosdtextparams (LsrDsMsg2pCtx *ctx, NvOSD_TextParams text_params)
{
  JsonObject *NvOSD_ColorParamsObj;
  JsonObject *NvOSD_TextParamsObj;
  JsonObject *NvOSD_FontParamsObj;

  NvOSD_TextParamsObj = json_object_new ();

  json_object_set_string_member (NvOSD_TextParamsObj, "display_text", text_params.display_text);
  json_object_set_int_member (NvOSD_TextParamsObj, "x_offset", text_params.x_offset);
  json_object_set_int_member (NvOSD_TextParamsObj, "y_offset", text_params.y_offset);

  /******************** font_params */
  NvOSD_FontParamsObj = generate_nvosdfontparams (ctx, text_params.font_params);
  json_object_set_object_member (NvOSD_TextParamsObj, "font_params", NvOSD_FontParamsObj);
  /******************** font_params */

  json_object_set_int_member (NvOSD_TextParamsObj, "set_bg_clr", text_params.set_bg_clr);

  /******************** text_bg_clr */
  NvOSD_ColorParamsObj = generate_nvosdcolorparams (ctx, text_params.text_bg_clr);
  json_object_set_object_member (NvOSD_TextParamsObj, "text_bg_clr", NvOSD_ColorParamsObj);
  /******************** text_bg_clr */

  return NvOSD_TextParamsObj;
}

static JsonObject*
generate_nvdsbasemeta (LsrDsMsg2pCtx *ctx, NvDsBaseMeta base_meta)
{
  JsonObject *NvDsBaseMetaObj;

  NvDsBaseMetaObj = json_object_new ();
  json_object_set_int_member (NvDsBaseMetaObj, "meta_type", base_meta.meta_type);

  return NvDsBaseMetaObj;
}

static JsonArray*
generate_nvdslabelinfolist_array (LsrDsMsg2pCtx *ctx, NvDsLabelInfoList *label_info_list)
{
  JsonArray *objectArray;
  JsonObject *NvDsLabelInfoObj;
  JsonObject *NvDsBaseMetaObj;

  objectArray = json_array_new ();

  NvDsLabelInfoList *l_label_info = NULL;
  NvDsLabelInfo *label_info = NULL;

  for (l_label_info = label_info_list; l_label_info; l_label_info = l_label_info->next) {

    label_info = (NvDsLabelInfo *) (l_label_info->data);

    NvDsLabelInfoObj = json_object_new ();

    NvDsBaseMetaObj = generate_nvdsbasemeta (ctx, label_info->base_meta);
    json_object_set_object_member (NvDsLabelInfoObj, "base_meta", NvDsBaseMetaObj);

    json_object_set_int_member (NvDsLabelInfoObj, "num_classes", label_info->num_classes);
    json_object_set_string_member (NvDsLabelInfoObj, "result_label", label_info->result_label);
    if (label_info->pResult_label) {
      json_object_set_string_member (NvDsLabelInfoObj, "pResult_label", label_info->pResult_label);
    }
    json_object_set_int_member (NvDsLabelInfoObj, "result_class_id", label_info->result_class_id);
    json_object_set_int_member (NvDsLabelInfoObj, "label_id", label_info->label_id);
    json_object_set_double_member (NvDsLabelInfoObj, "result_prob", label_info->result_prob);

    json_array_add_object_element (objectArray, NvDsLabelInfoObj);

  }

  return objectArray;
}

static JsonArray*
generate_nvdsclassifiermetalist_array (LsrDsMsg2pCtx *ctx, NvDsClassifierMetaList *classifier_meta_list)
{
  JsonArray *objectArray;
  JsonObject *NvDsClassifierMetaObj;
  JsonObject *NvDsBaseMetaObj;
  JsonArray *NvDsLabelInfoListObj;

  objectArray = json_array_new ();

  NvDsClassifierMetaList *l_classifier_meta = NULL;
  NvDsClassifierMeta *classifier_meta = NULL;

  for (l_classifier_meta = classifier_meta_list; l_classifier_meta; l_classifier_meta = l_classifier_meta->next) {

    classifier_meta = (NvDsClassifierMeta *) (l_classifier_meta->data);

    NvDsClassifierMetaObj = json_object_new ();

    NvDsBaseMetaObj = generate_nvdsbasemeta (ctx, classifier_meta->base_meta);
    json_object_set_object_member (NvDsClassifierMetaObj, "base_meta", NvDsBaseMetaObj);

    json_object_set_int_member (NvDsClassifierMetaObj, "num_labels", classifier_meta->num_labels);
    json_object_set_int_member (NvDsClassifierMetaObj, "unique_component_id", classifier_meta->unique_component_id);

    /********** label_info_list */
    NvDsLabelInfoListObj = generate_nvdslabelinfolist_array (ctx, classifier_meta->label_info_list);
    json_object_set_array_member (NvDsClassifierMetaObj, "classifier_meta_list", NvDsLabelInfoListObj);
    /********** label_info_list */

    json_array_add_object_element (objectArray, NvDsClassifierMetaObj);

  }

  return objectArray;
}

static JsonArray*
generate_nvdsobjectmetalist_array (LsrDsMsg2pCtx *ctx, NvDsObjectMetaList *obj_meta_list)
{
  JsonArray *objectArray;
  JsonObject *NvDsObjectMetaObj;
  JsonObject *NvDsBaseMetaObj;
  JsonObject *NvOSD_RectParamsObj;
  JsonObject *NvOSD_TextParamsObj;
  JsonArray *NvDsClassifierMetaListObj;

  objectArray = json_array_new ();

  NvDsObjectMetaList *l_obj_meta = NULL;
  NvDsObjectMeta *obj_meta = NULL;

  for (l_obj_meta = obj_meta_list; l_obj_meta; l_obj_meta = l_obj_meta->next) {

    obj_meta = (NvDsObjectMeta *) (l_obj_meta->data);

    NvDsObjectMetaObj = json_object_new ();

    if (ctx->payloadType == NVDS_PAYLOAD_DEEPSTREAM) {
      NvDsBaseMetaObj = generate_nvdsbasemeta (ctx, obj_meta->base_meta);
      json_object_set_object_member (NvDsObjectMetaObj, "base_meta", NvDsBaseMetaObj);
    }

    json_object_set_int_member (NvDsObjectMetaObj, "unique_component_id", obj_meta->unique_component_id);
    json_object_set_int_member (NvDsObjectMetaObj, "class_id", obj_meta->class_id);
    json_object_set_int_member (NvDsObjectMetaObj, "object_id", obj_meta->object_id);
    json_object_set_double_member (NvDsObjectMetaObj, "confidence", obj_meta->confidence);

    /********** rect_params */
    NvOSD_RectParamsObj = generate_nvosdrectparams (ctx, obj_meta->rect_params);
    json_object_set_object_member (NvDsObjectMetaObj, "rect_params", NvOSD_RectParamsObj);
    /********** rect_params */

    if (ctx->payloadType == NVDS_PAYLOAD_DEEPSTREAM) {
      /********** text_params */
      NvOSD_TextParamsObj = generate_nvosdtextparams (ctx, obj_meta->text_params);
      json_object_set_object_member (NvDsObjectMetaObj, "text_params", NvOSD_TextParamsObj);
      /********** text_params */
    }

    json_object_set_string_member (NvDsObjectMetaObj, "obj_label", obj_meta->obj_label);

    if (ctx->payloadType == NVDS_PAYLOAD_DEEPSTREAM) {
      /********** classifier_meta_list */
      NvDsClassifierMetaListObj = generate_nvdsclassifiermetalist_array (ctx, obj_meta->classifier_meta_list);
      json_object_set_array_member (NvDsObjectMetaObj, "classifier_meta_list", NvDsClassifierMetaListObj);
      /********** classifier_meta_list */
    }

    /*************************** TODO */
    /** list of pointers of type NvDsUserMeta */
    //NvDsUserMetaList *obj_user_meta_list;
    /** For additional user object info */
    //gint64 misc_obj_info[MAX_USER_FIELDS];
    /** For internal purpose only */
    //gint64 reserved[MAX_RESERVED_FIELDS];
    /*************************** TODO */

    json_array_add_object_element (objectArray, NvDsObjectMetaObj);

  }

  return objectArray;
}

static JsonArray*
generate_nvdisplaymetalist_array (LsrDsMsg2pCtx *ctx, NvDisplayMetaList *display_meta_list)
{
  JsonArray *objectArray;
  JsonObject *NvDsDisplayMetaObj;
  JsonObject *NvDsBaseMetaObj;

  objectArray = json_array_new ();

  NvDisplayMetaList *l_display_meta = NULL;
  NvDsDisplayMeta *display_meta = NULL;

  for (l_display_meta = display_meta_list; l_display_meta; l_display_meta = l_display_meta->next) {

    display_meta = (NvDsDisplayMeta *) (l_display_meta->data);

    NvDsDisplayMetaObj = json_object_new ();

    NvDsBaseMetaObj = generate_nvdsbasemeta (ctx, display_meta->base_meta);
    json_object_set_object_member (NvDsDisplayMetaObj, "base_meta", NvDsBaseMetaObj);

    json_object_set_int_member (NvDsDisplayMetaObj, "num_rects", display_meta->num_rects);
    json_object_set_int_member (NvDsDisplayMetaObj, "num_labels", display_meta->num_labels);
    json_object_set_int_member (NvDsDisplayMetaObj, "num_lines", display_meta->num_lines);

    /*************************** TODO */
    /** Structure containing the positional parameters to overlay borders
     * or semi-transparent rectangles as required by the user in the frame
     *  Refer NvOSD_RectParams from nvll_osd_struct.h
     */
    //NvOSD_RectParams rect_params[MAX_ELEMENTS_IN_DISPLAY_META];
    /** Text describing the user defined string can be overlayed using this
     * structure. @see NvOSD_TextParams from nvll_osd_struct.h */
    //NvOSD_TextParams text_params[MAX_ELEMENTS_IN_DISPLAY_META];
    /** parameters of the line of polygon that user can draw in the frame.
     *  e.g. to set ROI in the frame by specifying the lines.
     *  Refer NvOSD_RectParams from nvll_osd_struct.h */
    //NvOSD_LineParams line_params[MAX_ELEMENTS_IN_DISPLAY_META];
    /** user specific osd metadata*/
    //gint64 misc_osd_data[MAX_USER_FIELDS];
    /** for internal purpose */
    //gint64 reserved[MAX_RESERVED_FIELDS];
    /*************************** TODO */

    json_array_add_object_element (objectArray, NvDsDisplayMetaObj);

  }

  return objectArray;
}

static JsonObject*
generate_nvdsframemeta (LsrDsMsg2pCtx *ctx, NvDsFrameMeta *frame_meta)
{
  JsonObject *NvDsFrameMetaObj;
  JsonObject *NvDsBaseMetaObj;
  JsonArray *NvDsObjectMetaListObj;
  JsonArray *NvDisplayMetaListObj;

  uuid_t msgId;
  gchar msgIdStr[37];

  uuid_generate_random (msgId);
  uuid_unparse_lower(msgId, msgIdStr);


  // root object
  NvDsFrameMetaObj = json_object_new ();
  json_object_set_string_member (NvDsFrameMetaObj, "messageid", msgIdStr);
  if (ctx->payloadType == NVDS_PAYLOAD_DEEPSTREAM) {
    json_object_set_string_member (NvDsFrameMetaObj, "dsversion", "4.0");
  }

  if (ctx->payloadType == NVDS_PAYLOAD_DEEPSTREAM) {
    NvDsBaseMetaObj = generate_nvdsbasemeta (ctx, frame_meta->base_meta);
    json_object_set_object_member (NvDsFrameMetaObj, "base_meta", NvDsBaseMetaObj);
  }

  if (ctx->payloadType == NVDS_PAYLOAD_DEEPSTREAM) {
    json_object_set_int_member (NvDsFrameMetaObj, "pad_index", frame_meta->pad_index);
    json_object_set_int_member (NvDsFrameMetaObj, "batch_id", frame_meta->batch_id);
    json_object_set_int_member (NvDsFrameMetaObj, "buf_pts", frame_meta->buf_pts);
    json_object_set_int_member (NvDsFrameMetaObj, "ntp_timestamp", frame_meta->ntp_timestamp);
  }
  json_object_set_int_member (NvDsFrameMetaObj, "timestamp", std::time(0));
  json_object_set_int_member (NvDsFrameMetaObj, "frame_num", frame_meta->frame_num);
  json_object_set_int_member (NvDsFrameMetaObj, "source_id", frame_meta->source_id);
  if (ctx->payloadType == NVDS_PAYLOAD_DEEPSTREAM) {
    json_object_set_int_member (NvDsFrameMetaObj, "num_surfaces_per_frame", frame_meta->num_surfaces_per_frame);
    json_object_set_int_member (NvDsFrameMetaObj, "source_frame_width", frame_meta->source_frame_width);
    json_object_set_int_member (NvDsFrameMetaObj, "source_frame_height", frame_meta->source_frame_height);
    json_object_set_int_member (NvDsFrameMetaObj, "surface_type", frame_meta->surface_type);
    json_object_set_int_member (NvDsFrameMetaObj, "surface_index", frame_meta->surface_index);
  }
  json_object_set_int_member (NvDsFrameMetaObj, "num_obj_meta", frame_meta->num_obj_meta);
  if (ctx->payloadType == NVDS_PAYLOAD_DEEPSTREAM) {
    json_object_set_boolean_member (NvDsFrameMetaObj, "bInferDone", frame_meta->bInferDone);
  }

  NvDsObjectMetaListObj = generate_nvdsobjectmetalist_array (ctx, frame_meta->obj_meta_list);
  json_object_set_array_member (NvDsFrameMetaObj, "obj_meta_list", NvDsObjectMetaListObj);

  if (ctx->payloadType == NVDS_PAYLOAD_DEEPSTREAM) {
    NvDisplayMetaListObj = generate_nvdisplaymetalist_array (ctx, frame_meta->display_meta_list);
    json_object_set_array_member (NvDsFrameMetaObj, "display_meta_list", NvDisplayMetaListObj);
  }

  /*************************** TODO */
  /** list of pointers of type “NvDsUserMeta” in use for the given frame */
  //NvDsUserMetaList *frame_user_meta_list;
  /** For additional user frame info */
  //gint64 misc_frame_info[MAX_USER_FIELDS];
  /**For internal purpose */
  //gint64 reserved[MAX_RESERVED_FIELDS];
  /*************************** TODO */

  return NvDsFrameMetaObj;
}

static gchar*
generate_schema_message (LsrDsMsg2pCtx *ctx, NvDsFrameMeta *frame_meta)
{
  JsonNode *rootNode;
  JsonObject *NvDsFrameMetaObj;
  gchar *message;

  NvDsFrameMetaObj = generate_nvdsframemeta (ctx, frame_meta);

  // root object
//  rootObj = json_object_new ();
//  json_object_set_string_member (rootObj, "messageid", msgIdStr);
//  json_object_set_string_member (rootObj, "mdsversion", "1.0");
//  json_object_set_string_member (rootObj, "@timestamp", meta->ts);

  rootNode = json_node_new (JSON_NODE_OBJECT);
  json_node_set_object (rootNode, NvDsFrameMetaObj);

  message = json_to_string (rootNode, TRUE);
  json_node_free (rootNode);
  json_object_unref (NvDsFrameMetaObj);

  return message;
}

LsrDsMsg2pCtx* lsrds_msg2p_ctx_create (NvDsPayloadType type)
{
  LsrDsMsg2pCtx *ctx = NULL;
  bool retVal = true;

  ctx = new LsrDsMsg2pCtx;

  ctx->payloadType = type;

  if (!retVal) {
    cout << "Error in creating instance" << endl;

    if (ctx) {
      delete ctx;
      ctx = NULL;
    }
  }
  return ctx;
}

void lsrds_msg2p_ctx_destroy (LsrDsMsg2pCtx *ctx)
{
  delete ctx;
}

NvDsPayload*
lsrds_msg2p_generate (LsrDsMsg2pCtx *ctx, NvDsFrameMeta *frame_meta, guint size)
{
  gchar *message = NULL;
  gint len = 0;
  NvDsPayload *payload = (NvDsPayload *) g_malloc0 (sizeof (NvDsPayload));

  payload->payload = NULL;

  if (ctx->payloadType != NVDS_PAYLOAD_CUSTOM) {
    message = generate_schema_message (ctx, frame_meta);
    if (message) {
      len = strlen (message);
      // Remove '\0' character at the end of string and just copy the content.
      payload->payload = g_memdup (message, len);
      payload->payloadSize = len;
      g_free (message);
    }
  }

  return payload;
}

void
lsrds_msg2p_release (LsrDsMsg2pCtx *ctx, NvDsPayload *payload)
{
  g_free (payload->payload);
  g_free (payload);
}
