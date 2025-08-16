#ifndef INKSCAPE_TRACE_H
#define INKSCAPE_TRACE_H

#include <functional>
#include <gdkmm/pixbuf.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <2geom/pathvector.h>

namespace Inkscape {

namespace Async {
// 简单进度类（非模板版本）
class Progress {
public:
    void throw_if_cancelled() const {}
    void report_or_throw(double) {}
};
}

namespace Trace {

// 追踪结果项
struct TraceResultItem {
  TraceResultItem(std::string style_, Geom::PathVector path_)
      : style(std::move(style_)), path(std::move(path_)) {}

  std::string style;
  Geom::PathVector path;
};

// 追踪结果
using TraceResult = std::vector<TraceResultItem>;

/**
 * A generic interface for plugging different autotracers into Inkscape.
 * 一个通用的接口，用于将不同的自动追踪器插入到 Inkscape 中。
 */
// 追踪引擎
class TracingEngine {
public:
  TracingEngine() = default;
  virtual ~TracingEngine() = default;

  /**
   * This is the working method of this interface, and all implementing classes.
   * Take a GdkPixbuf, trace it, and return a style attribute and the path data
   * that is compatible with the d="" attribute of an SVG <path> element.
   *
   * This function will be called off-main-thread, so is required to be
   * thread-safe. The lack of const however indicates that it is not required to
   * be re-entrant.
   *
   * 这个函数将返回一个追踪结果，其中包含一个样式属性和一个路径数据，
   * 这个路径数据是与 SVG <path> 元素的 d="" 属性兼容的。
   *
   * 这个函数将在后台线程中被调用，因此需要是线程安全的。
   * 由于缺少 const，它不是可重入的。
   *
   */
  virtual TraceResult trace(Glib::RefPtr<Gdk::Pixbuf> const &pixbuf,
                            Async::Progress &progress) = 0;

  /**
   * Generate a quick preview without any actual tracing. Like trace(), this
   * must be thread-safe.
   */
  virtual Glib::RefPtr<Gdk::Pixbuf>
  preview(Glib::RefPtr<Gdk::Pixbuf> const &pixbuf) = 0;

  /**
   * Return true if the user should be checked with before tracing because the
   * image is too big.
   */
  virtual bool check_image_size(Geom::IntPoint const &size) const {
    return false;
  }
};

// 追踪Future
namespace detail {
struct TraceFutureCreate;
}

// 追踪Future（简化版本）
class TraceFuture {
public:
  // 取消追踪
  void cancel() {
    cancelled = true;
  }
  explicit operator bool() const { return !cancelled; }

private:
  bool cancelled = false;
  // 追踪Future创建者
  friend class detail::TraceFutureCreate;
};

/**
 * Launch an asynchronous trace operation taking as input \a engine and \a
 * sioxEnabled. If this returns null, the task failed to launch and no further
 * action will be taken. Otherwise, a background task is launched which will
 * call \a onprogress some number of times followed by \a onfinished exactly
 * once. Both callbacks are invoked from the GTK main loop.
 *
 * 启动一个异步追踪操作，输入 \a engine 和 \a sioxEnabled。
 * 如果返回 null，则追踪任务失败，不会采取进一步行动。
 * 否则，将启动一个后台任务，该任务将调用 \a onprogress 一些次数，
 * 然后调用 \a onfinished 一次。两个回调都从 GTK 主循环中调用。
 *
 */
TraceFuture trace(std::unique_ptr<TracingEngine> engine, bool sioxEnabled,
                  std::function<void(double)> onprogress,
                  std::function<void()> onfinished);

/**
 * Similar to \a trace(), but computes the preview and passes it to \a
 * onfinished when done.
 *
 * 类似于 \a trace()，但计算预览并将其传递给 \a onfinished 完成时。
 *
 */
TraceFuture preview(std::unique_ptr<TracingEngine> engine, bool sioxEnabled,
                    std::function<void(Glib::RefPtr<Gdk::Pixbuf>)> onfinished);

} // namespace Trace
} // namespace Inkscape

#endif // INKSCAPE_TRACE_H
