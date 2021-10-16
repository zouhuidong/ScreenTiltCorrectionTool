///////////////////////////////////////////
//
//	屏幕倾斜实时矫正工具
//
//	VS 2019 + EasyX 20210730
// 
//	huidong <huidong_mail@163.com>
//	2021.10.16
//

#include <Windows.h>
#include <WinUser.h>
#include <easyx.h>

// 点是否位于矩形内
#define isInRect(x, y, rct) (x >= rct.left && x <= rct.right && y >= rct.top && y <= rct.bottom)

// 存储整个屏幕的大小信息（多显示器）
struct ScreenSize
{
	int left;	// 多显示器的左上角 x 坐标
	int top;	// 多显示器的左上角 y 坐标
	int w;	// 多显示器的总和宽度
	int h;	// 多显示器的总和高度
};

// 屏幕区域图像
struct AreaImage
{
	IMAGE img;	// 该区域的图像
	RECT rct;	// 该区域位置
};

// 获取多显示器大小信息
ScreenSize GetScreenSize()
{
	int left = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int top = GetSystemMetrics(SM_YVIRTUALSCREEN);
	int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	return { left,top,w,h };
}

// 抓取桌面图像到 *pimg 对象中
void CaptureDesktop(IMAGE* pimg)
{
	ScreenSize s = GetScreenSize();
	Resize(pimg, s.w, s.h);
	HDC srcDC = ::GetDC(NULL);
	HDC dstDC = GetImageHDC(pimg);
	BitBlt(dstDC, 0, 0, s.w, s.h, srcDC, s.left, s.top, SRCCOPY);
}

// 图片拉伸
// width, height 拉伸后的图片大小
// imgResize 原图像
void ImageToSize(int width, int height, IMAGE* img)
{
	IMAGE* pOldImage = GetWorkingImage();
	SetWorkingImage(img);

	IMAGE temp_image(width, height);

	StretchBlt(
		GetImageHDC(&temp_image), 0, 0, width, height,
		GetImageHDC(img), 0, 0,
		getwidth(), getheight(),
		SRCCOPY
	);

	Resize(img, width, height);
	putimage(0, 0, &temp_image);

	SetWorkingImage(pOldImage);
}

// 介绍菜单
void Menu()
{
	setbkcolor(WHITE);
	cleardevice();
	settextcolor(BLACK);

	settextstyle(32, 0, L"system");
	outtextxy(20, 20, L"屏幕画面倾斜实时矫正工具");
	settextstyle(28, 0, L"system");
	outtextxy(20, 60, L"Real-time tilt correction tool       by huidong 2021.10.16");

	settextstyle(14, 0, L"宋体");
	outtextxy(30, 430, L"设计需求：（惭愧，没条件去电影院的我……）看 TC 电影的时候，录制者的相机角度倾斜了，");
	outtextxy(30, 454, L"特制此工具实时矫正屏幕倾斜画面。");

	RECT btn = { 200, 200, 420, 250 };
	setlinecolor(BLACK);
	rectangle(btn.left, btn.top, btn.right, btn.bottom);
	drawtext(L"实时矫正 >>", &btn, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

	while (true)
	{
		ExMessage msg = getmessage(EM_MOUSE);
		if (isInRect(msg.x, msg.y, btn) && msg.message == WM_LBUTTONUP)
		{
			break;
		}
	}
}

/**
 * @brief 获取图像在指定最大长宽时的缩放比例
 * @param img 图像指针
 * @param lw 图像的最大宽度
 * @param lh 图像的最大高度
 * @return 返回图像的缩放比例
*/
double GetImageRatioInSize(IMAGE* img, int lw, int lh)
{
	double ratio1 = (double)lw / img->getwidth();
	double ratio2 = (double)lh / img->getheight();
	return ratio1 < ratio2 ? ratio1 : ratio2;
}

// 以点为中心制造矩形
RECT MakeRectFromPoint(int x, int y, int radius)
{
	return { x - radius,y - radius,x + radius,y + radius };
}

// 重绘矩形
void DrawRect(int img_x, int img_y, IMAGE* img, RECT rct, int nRectRadius, int imgarea_w, int imgarea_h)
{
	setlinestyle(PS_SOLID, 3);
	clearrectangle(img_x, img_y, img_x + imgarea_w, img_y + imgarea_h);
	putimage(img_x, img_y, img);
	rectangle(rct.left, rct.top, rct.right, rct.bottom);
	setlinestyle(PS_SOLID, 2);
	fillcircle(rct.left, rct.top, nRectRadius);
	fillcircle(rct.right, rct.bottom, nRectRadius);
}

// 调整窗口大小
void ResizeWindow(int w, int h)
{
	closegraph();
	initgraph(w, h);
	setbkcolor(WHITE);
	cleardevice();
}

// 选择旋转区域，返回此区域的位置和图像
AreaImage SelectRect(IMAGE* img)
{
	// 设备宽高
	int dw = 1080;
	int dh = 420;

	// 图像区域可用宽高
	int imgarea_w = dw;
	int imgarea_h = dh - 60;

	// 图像输出偏移
	int img_x = 0, img_y = 24;

	// 图像显示区域
	RECT rctImageArea = { img_x,img_y,img_x + imgarea_w ,img_y + imgarea_h };

	// 调整
	ResizeWindow(dw, dh);

	IMAGE imgResize = *img;

	// 计算缩放比例
	double ratio = GetImageRatioInSize(img, rctImageArea.right - rctImageArea.left, rctImageArea.bottom - rctImageArea.top);
	ImageToSize((int)(img->getwidth() * ratio), (int)(img->getheight() * ratio), &imgResize);

	settextcolor(BLACK);
	settextstyle(24, 0, L"宋体");
	outtextxy(0, 0, L"请拖动矩形，选择你要实时矫正的区域");

	// 按钮
	RECT btn = { dw - 140, dh - 30, dw - 3, dh - 3 };
	setlinecolor(BLACK);
	settextstyle(16, 0, L"宋体");
	rectangle(btn.left, btn.top, btn.right, btn.bottom);
	drawtext(L"下一步 >>", &btn, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

	// 用户设置的区域
	RECT rct = { 100,100,200,200 };

	setlinecolor(LIGHTGREEN);
	setfillcolor(YELLOW);

	const int nRectRadius = 5;

	DrawRect(img_x, img_y, &imgResize, rct, nRectRadius, imgarea_w, imgarea_h);

	// 用户操作
	ExMessage msg;
	int nPressId = -1;
	while (true)
	{
		// 矩形的两个顶点
		RECT vertex[2];
		vertex[0] = MakeRectFromPoint(rct.left, rct.top, nRectRadius);
		vertex[1] = MakeRectFromPoint(rct.right, rct.bottom, nRectRadius);

		// 响应顶点拖动、按钮点击
		if (peekmessage(&msg, EM_MOUSE))
		{
			// 顶点只能在图像区域内调整
			if (isInRect(msg.x, msg.y, rctImageArea))
			{
				if ((isInRect(msg.x, msg.y, vertex[0]) && msg.lbutton) || nPressId == 0)
				{
					rct.left = msg.x;
					rct.top = msg.y;
					nPressId = 0;

					DrawRect(img_x, img_y, &imgResize, rct, nRectRadius, imgarea_w, imgarea_h);
				}
				else if ((isInRect(msg.x, msg.y, vertex[1]) && msg.lbutton) || nPressId == 1)
				{
					rct.right = msg.x;
					rct.bottom = msg.y;
					nPressId = 1;

					DrawRect(img_x, img_y, &imgResize, rct, nRectRadius, imgarea_w, imgarea_h);
				}
			}

			if (msg.message == WM_LBUTTONUP)
			{
				nPressId = -1;

				if (isInRect(msg.x, msg.y, btn))
				{
					break;
				}
			}

		}
	}

	// 根据比例复原矩形坐标

	// 顺序调整
	if (rct.left > rct.right)
	{
		int t = rct.left;
		rct.left = rct.right;
		rct.right = t;
	}
	if (rct.top > rct.bottom)
	{
		int t = rct.top;
		rct.top = rct.bottom;
		rct.bottom = t;
	}

	rct.left -= img_x;
	rct.top -= img_y;
	rct.right -= img_x;
	rct.bottom -= img_y;
	rct.left = (long)(rct.left / ratio);
	rct.top = (long)(rct.top / ratio);
	rct.right = (long)(rct.right / ratio);
	rct.bottom = (long)(rct.bottom / ratio);
	int rw = rct.right - rct.left;
	int rh = rct.bottom - rct.top;

	// 裁剪矩形内图像
	SetWorkingImage(img);
	IMAGE cut;
	getimage(&cut, rct.left, rct.top, rw, rh);
	SetWorkingImage();

	return { cut,rct };
}

// 设置倾斜角
double SetRadian(IMAGE* img)
{
	ResizeWindow(640, 480);

	settextstyle(24, 0, L"宋体");
	settextcolor(BLACK);
	outtextxy(0, 0, L"请设置此区域的倾斜角");

	RECT btn[3] = { { 210, 420, 240, 450 },
					{ 310, 420, 340, 450 },
					{ getwidth() - 200, getheight() - 50, getwidth() - 3, getheight() - 3 } };

	setlinecolor(BLACK);
	for (int i = 0; i < 3; i++)
	{
		fillrectangle(btn[i].left, btn[i].top, btn[i].right, btn[i].bottom);
	}

	settextstyle(12, 0, L"宋体");
	drawtext(L"←", &btn[0], DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	drawtext(L"→", &btn[1], DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	drawtext(L"开始实时矫正 >>", &btn[2], DT_CENTER | DT_SINGLELINE | DT_VCENTER);

	BeginBatchDraw();

	double radian = 0;
	int nPressId = -1;
	while (true)
	{
		// 重绘
		clearrectangle(0, 24, getwidth(), 419);

		IMAGE rotated;
		rotateimage(&rotated, img, radian, WHITE, true);

		double ratio = GetImageRatioInSize(&rotated, getwidth(), getheight() - 100);
		ImageToSize((int)(rotated.getwidth() * ratio), (int)(rotated.getheight() * ratio), &rotated);
		putimage(0, 30, &rotated);

		FlushBatchDraw();

		// 消息响应
		while (true)
		{
			ExMessage msg;
			bool flag = false;
			if (peekmessage(&msg, EM_MOUSE) || nPressId > -1)
			{
				if (nPressId > -1 && !msg.lbutton)
				{
					nPressId = -1;
				}

				if ((isInRect(msg.x, msg.y, btn[0]) && msg.lbutton) || nPressId == 0)
				{
					radian -= 0.01;
					nPressId = 0;
					flag = true;
				}
				else if ((isInRect(msg.x, msg.y, btn[1]) && msg.lbutton) || nPressId == 1)
				{
					radian += 0.01;
					nPressId = 1;
					flag = true;
				}
				else if (isInRect(msg.x, msg.y, btn[2]) && msg.message == WM_LBUTTONUP)
				{
					goto end;
				}

				if (flag)
				{
					// 减速，防止调整过快
					Sleep(50);
					break;
				}
			}
		}

	}

end:

	EndBatchDraw();

	return radian;
}

// 初始化一个全屏窗口
void InitFullScreenWindow()
{
	initgraph(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	SetWindowLong(GetHWnd(), GWL_STYLE, GetWindowLong(GetHWnd(), GWL_STYLE) & ~WS_CAPTION);
	SetWindowPos(GetHWnd(), HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE);
}

// 倾斜矫正
void TiltCorrect(RECT rct, double radian)
{
	int w1 = rct.right - rct.left;
	int h1 = rct.bottom - rct.top;

	// 获取旋转后图像大小
	IMAGE temp[2];
	temp[1].Resize(w1, h1);
	rotateimage(&temp[0], &temp[1], radian, WHITE, true);
	int w2 = temp[0].getwidth();
	int h2 = temp[0].getheight();

	const int nAddHeight = 40;

	// 重设窗口大小
	ResizeWindow(w2, h2 + nAddHeight);

	RECT btn[2] = { { 250, h2 + 5, 380, h2 + 35 },
					{ 400, h2 + 5, 500, h2 + 35 } };

	ExMessage msg;
	bool bRedrawAll = false;
	bool bRedrawControl = true;
	bool isFullWindow = false;
	bool isFastMode = false;
	double zoom = 1;
	int offset_x = 0, offset_y = 0;
	bool lbutton = false;
	int mouse_x, mouse_y;
	while (true)
	{
		// 全部重绘
		if (bRedrawAll)
		{
			cleardevice();
			bRedrawControl = true;
			bRedrawAll = false;
		}

		// 图像绘制
		{
			// 截图
			IMAGE img, imgRect, imgRotate;
			CaptureDesktop(&img);

			// 裁剪
			SetWorkingImage(&img);
			getimage(&imgRect, rct.left, rct.top, w1, h1);
			SetWorkingImage();

			// 旋转（极速模式下使用低质量旋转）
			rotateimage(&imgRotate, &imgRect, radian, WHITE, true, !isFastMode);

			// 若有缩放则进行计算
			if ((int)(zoom * 10) != 10)
			{
				ImageToSize((int)(imgRotate.getwidth() * zoom), (int)(imgRotate.getheight() * zoom), &imgRotate);
			}

			// 绘制
			putimage(offset_x, offset_y, &imgRotate);
		}

		// 重绘控件
		if (bRedrawControl)
		{
			setlinecolor(BLACK);
			settextstyle(14, 0, L"宋体");
			settextcolor(BLACK);

			for (int i = 0; i < 2; i++)
			{
				fillrectangle(btn[i].left, btn[i].top, btn[i].right, btn[i].bottom);
			}

			outtextxy(10, h2 + 5, L"鼠标可拖动图像，滚轮操纵图像缩放");
			drawtext(L"全屏（ESC 退出）", &btn[0], DT_CENTER | DT_SINGLELINE | DT_VCENTER);

			if (isFastMode)
			{
				drawtext(L"切换正常模式", &btn[1], DT_CENTER | DT_SINGLELINE | DT_VCENTER);
			}
			else
			{
				drawtext(L"开启极速模式", &btn[1], DT_CENTER | DT_SINGLELINE | DT_VCENTER);
			}

			line(0, h2 + 1, getwidth(), h2 + 1);

			bRedrawControl = false;
		}

		// 消息处理
		if (peekmessage(&msg, EM_MOUSE))
		{
			// 全屏下不响应按键
			if (!isFullWindow)
			{
				if (isInRect(msg.x, msg.y, btn[0]) && msg.message == WM_LBUTTONUP)
				{
					InitFullScreenWindow();
					isFullWindow = true;
				}
				else if (isInRect(msg.x, msg.y, btn[1]) && msg.message == WM_LBUTTONUP)
				{
					isFastMode = !isFastMode;
					bRedrawControl = true;	// 重绘控件
				}
			}

			// 图像缩放
			if (msg.message == WM_MOUSEWHEEL)
			{
				zoom += msg.wheel * 0.0003;

				// 缩放限制
				if (zoom < 0)
				{
					zoom = 0.05;
				}
				else if (zoom > 20)
				{
					zoom = 20;
				}
			}

			// 图像移动
			if (msg.lbutton && !lbutton)
			{
				lbutton = true;
				mouse_x = msg.x;
				mouse_y = msg.y;
			}
			else if (lbutton)
			{
				if (msg.lbutton)
				{
					offset_x += msg.x - mouse_x;
					offset_y += msg.y - mouse_y;
					mouse_x = msg.x;
					mouse_y = msg.y;
				}
				else
				{
					lbutton = false;
				}
			}
		}

		if (peekmessage(&msg, EM_KEY))
		{
			// 退出全屏
			if (isFullWindow && msg.vkcode == VK_ESCAPE)
			{
				ResizeWindow(w2, h2 + nAddHeight);
				isFullWindow = false;
				bRedrawControl = true;	// 重绘控件
			}
		}

		Sleep(1);
	}

}

// 主函数
int main()
{
	IMAGE img;
	CaptureDesktop(&img);

	initgraph(640, 480);

	Menu();
	AreaImage aimg = SelectRect(&img);
	double radian = SetRadian(&aimg.img);
	TiltCorrect(aimg.rct, radian);

	closegraph();
	return 0;
}