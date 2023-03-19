#include <gst/gst.h>
#include <gst/video/video.h>
#include <opencv2/opencv.hpp>

GST_DEBUG_CATEGORY_STATIC (gst_background_subtraction_plugin_debug);
#define GST_CAT_DEFAULT gst_background_subtraction_plugin_debug

#define GST_TYPE_BACKGROUND_SUBTRACTION_PLUGIN \
  (gst_background_subtraction_plugin_get_type())
#define GST_BACKGROUND_SUBTRACTION_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_BACKGROUND_SUBTRACTION_PLUGIN,GstBackgroundSubtractionPlugin))
#define GST_BACKGROUND_SUBTRACTION_PLUGIN_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_BACKGROUND_SUBTRACTION_PLUGIN,GstBackgroundSubtractionPluginClass))
#define GST_IS_BACKGROUND_SUBTRACTION_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_BACKGROUND_SUBTRACTION_PLUGIN))
#define GST_IS_BACKGROUND_SUBTRACTION_PLUGIN_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_BACKGROUND_SUBTRACTION_PLUGIN))
#define GST_BACKGROUND_SUBTRACTION_PLUGIN_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS((obj),GST_TYPE_BACKGROUND_SUBTRACTION_PLUGIN,GstBackgroundSubtractionPluginClass))

typedef struct _GstBackgroundSubtractionPlugin GstBackgroundSubtractionPlugin;
typedef struct _GstBackgroundSubtractionPluginClass GstBackgroundSubtractionPluginClass;

struct _GstBackgroundSubtractionPlugin
{
  GstVideoFilter base;

  // Add any private variables here
  cv::Ptr<cv::BackgroundSubtractor> subtractor;
};

struct _GstBackgroundSubtractionPluginClass
{
  GstVideoFilterClass base_class;
};

GType gst_background_subtraction_plugin_get_type (void);

G_DEFINE_TYPE_WITH_CODE (GstBackgroundSubtractionPlugin, gst_background_subtraction_plugin,
                         GST_TYPE_VIDEO_FILTER,
                         GST_DEBUG_CATEGORY_INIT (gst_background_subtraction_plugin_debug,
                             "backgroundsubtractionplugin", 0,
                             "BackgroundSubtraction Plugin"));

static gboolean
gst_background_subtraction_plugin_start (GstBaseTransform * trans)
{
  GstBackgroundSubtractionPlugin *plugin = GST_BACKGROUND_SUBTRACTION_PLUGIN (trans);
  plugin->subtractor = cv::createBackgroundSubtractorMOG2();
  return TRUE;
}

static gboolean
gst_background_subtraction_plugin_stop (GstBaseTransform * trans)
{
  GstBackgroundSubtractionPlugin *plugin = GST_BACKGROUND_SUBTRACTION_PLUGIN (trans);
  plugin->subtractor.release();
  return TRUE;
}

static GstFlowReturn
gst_background_subtraction_plugin_transform_frame (GstVideoFilter * filter, GstVideoFrame * frame)
{
  GstBackgroundSubtractionPlugin *plugin = GST_BACKGROUND_SUBTRACTION_PLUGIN (filter);
  cv::Mat input_mat(frame->height, frame->width, CV_8UC3, GST_VIDEO_FRAME_PLANE_DATA(frame, 0));
  cv::Mat output_mat;
  plugin->subtractor->apply(input_mat, output_mat, 0.01);
  cv::cvtColor(output_mat, output_mat, cv::COLOR_GRAY2BGR);
  memcpy(GST_VIDEO_FRAME_PLANE_DATA(frame, 0), output_mat.data, frame
  ->size);

return GST_FLOW_OK;
}

static void
gst_background_subtraction_plugin_class_init (GstBackgroundSubtractionPluginClass * klass)
{
GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);
GstVideoFilterClass *video_filter_class = GST_VIDEO_FILTER_CLASS (klass);

base_transform_class->start = GST_DEBUG_FUNCPTR (gst_background_subtraction_plugin_start);
base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_background_subtraction_plugin_stop);

video_filter_class->transform_frame = GST_DEBUG_FUNCPTR (gst_background_subtraction_plugin_transform_frame);
}

static void
gst_background_subtraction_plugin_init (GstBackgroundSubtractionPlugin * plugin)
{
// Initialize any private variables here
}

static GType
gst_background_subtraction_plugin_get_type (void)
{
static GType gst_background_subtraction_plugin_type = 0;

if (!gst_background_subtraction_plugin_type) {
static const GTypeInfo gst_background_subtraction_plugin_info = {
sizeof (GstBackgroundSubtractionPluginClass),
NULL,
NULL,
(GClassInitFunc) gst_background_subtraction_plugin_class_init,
NULL,
NULL,
sizeof (GstBackgroundSubtractionPlugin),
0,
(GInstanceInitFunc) gst_background_subtraction_plugin_init,
};
