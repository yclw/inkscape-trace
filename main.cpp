#include "ppotrace.h"
#include "svg/svg.h"
#include <iostream>
#include <fstream>
#include <cstring>  // for memcpy
#include <opencv2/opencv.hpp>
#include <gdkmm/pixbuf.h>
#include <gtkmm.h>

using namespace Inkscape::Trace::Potrace;
using namespace Inkscape::Trace;

// 将OpenCV Mat转换为GdkPixbuf
Glib::RefPtr<Gdk::Pixbuf> matToGdkPixbuf(const cv::Mat& mat) {
    cv::Mat rgb_mat;
    if (mat.channels() == 3) {
        cv::cvtColor(mat, rgb_mat, cv::COLOR_BGR2RGB);
    } else {
        cv::cvtColor(mat, rgb_mat, cv::COLOR_GRAY2RGB);
    }
    
    // 确保数据连续
    if (!rgb_mat.isContinuous()) {
        rgb_mat = rgb_mat.clone();
    }
    
    // 🔧 修复：创建新的GdkPixbuf并复制数据，而不是引用临时数据
    auto pixbuf = Gdk::Pixbuf::create(Gdk::Colorspace::RGB, false, 8, 
                                     rgb_mat.cols, rgb_mat.rows);
    
    // 复制数据到GdkPixbuf的内存空间
    auto dst_data = pixbuf->get_pixels();
    auto src_data = rgb_mat.data;
    int src_rowstride = rgb_mat.cols * 3;
    int dst_rowstride = pixbuf->get_rowstride();
    
    for (int y = 0; y < rgb_mat.rows; y++) {
        memcpy(dst_data + y * dst_rowstride, 
               src_data + y * src_rowstride, 
               src_rowstride);
    }
    
    return pixbuf;
}

// 生成真正的矢量化SVG文件
void generateVectorSvg(const TraceResult& traceResult, 
                       const std::string& filename, int width, int height) {
    std::ofstream file(filename);
    
    // 设置C locale避免数字中的千位分隔符
    file.imbue(std::locale::classic());
    
    file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
    file << "<svg width=\"" << width << "\" height=\"" << height << "\" ";
    file << "viewBox=\"0 0 " << width << " " << height << "\" ";
    file << "xmlns=\"http://www.w3.org/2000/svg\" ";
    file << "preserveAspectRatio=\"xMidYMid meet\">" << std::endl;
    
    // 反转图层顺序，确保第一层在最底部，最后一层在最顶部
    for (auto it = traceResult.rbegin(); it != traceResult.rend(); ++it) {
        if (it->path.empty()) continue;
        
        // 使用svg/svg.h中声明的函数来生成路径字符串
        std::string pathData = sp_svg_write_path(it->path);
        
        file << "  <path d=\"" << pathData << "\" style=\"" << it->style << "\" />" << std::endl;
    }
    
    file << "</svg>" << std::endl;
    file.close();
}

int main(int argc, char* argv[]) {
    try {
        // 正确初始化GTK和GLib
        auto app = Gtk::Application::create();
        // 或者更简单的方式
        Glib::init();
        
        std::cout << "正在加载图像 test.jpg..." << std::endl;
        
        // 只使用OpenCV读取图像
        cv::Mat image = cv::imread("test.jpg");
        if (image.empty()) {
            std::cerr << "无法加载图像 test.jpg" << std::endl;
            return 1;
        }
        
        std::cout << "图像尺寸: " << image.cols << "x" << image.rows << std::endl;
        
        // 转换为GdkPixbuf
        std::cout << "转换图像格式..." << std::endl;
        auto pixbuf = matToGdkPixbuf(image);
        if (!pixbuf) {
            std::cerr << "无法转换图像格式" << std::endl;
            return 1;
        }
        
        std::cout << "创建Potrace追踪引擎..." << std::endl;
        
        std::cout << "开始矢量化追踪..." << std::endl;
        
        // 创建进度对象
        Inkscape::Async::Progress progress;
        
        // 执行Potrace追踪
        TraceResult traceResult;
        
        // 创建Potrace追踪引擎
        PotraceTracingEngine engine(
            TraceType::QUANT_COLOR,    // 量化颜色模式
            false,                     // 不反转
            6,                         // 量化颜色数：6层
            0.35,                      // 亮度阈值
            0.0,                       // 亮度地板
            0.55,                      // 边缘检测阈值
            6,                         // 多扫描颜色数：6层
            true,                      // 多扫描堆叠：Stack模式
            false,                     // 不平滑
            true                      // 移除背景
        );
        
        traceResult = engine.trace(pixbuf, progress);
        
        std::cout << "追踪完成，共生成 " << traceResult.size() << " 个矢量路径层" << std::endl;
        
        std::cout << "生成矢量化SVG文件..." << std::endl;
        
        // 生成真正的矢量化SVG文件
        generateVectorSvg(traceResult, "output.svg", image.cols, image.rows);
        
        std::cout << "矢量化SVG文件已保存为 output.svg" << std::endl;

        std::cout << "文件大小：" << std::filesystem::file_size("output.svg") << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
}