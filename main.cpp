///////////////////////////////////////////
//
//	��Ļ��бʵʱ��������
//
//	VS 2019 + EasyX 20210730
// 
//	huidong <huidong_mail@163.com>
//	2021.10.16
//

#include <Windows.h>
#include <WinUser.h>
#include <easyx.h>

// ���Ƿ�λ�ھ�����
#define isInRect(x, y, rct) (x >= rct.left && x <= rct.right && y >= rct.top && y <= rct.bottom)

// �洢������Ļ�Ĵ�С��Ϣ������ʾ����
struct ScreenSize
{
	int left;	// ����ʾ�������Ͻ� x ����
	int top;	// ����ʾ�������Ͻ� y ����
	int w;	// ����ʾ�����ܺͿ��
	int h;	// ����ʾ�����ܺ͸߶�
};

// ��Ļ����ͼ��
struct AreaImage
{
	IMAGE img;	// �������ͼ��
	RECT rct;	// ������λ��
};

// ��ȡ����ʾ����С��Ϣ
ScreenSize GetScreenSize()
{
	int left = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int top = GetSystemMetrics(SM_YVIRTUALSCREEN);
	int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	return { left,top,w,h };
}

// ץȡ����ͼ�� *pimg ������
void CaptureDesktop(IMAGE* pimg)
{
	ScreenSize s = GetScreenSize();
	Resize(pimg, s.w, s.h);
	HDC srcDC = ::GetDC(NULL);
	HDC dstDC = GetImageHDC(pimg);
	BitBlt(dstDC, 0, 0, s.w, s.h, srcDC, s.left, s.top, SRCCOPY);
}

// ͼƬ����
// width, height ������ͼƬ��С
// imgResize ԭͼ��
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

// ���ܲ˵�
void Menu()
{
	setbkcolor(WHITE);
	cleardevice();
	settextcolor(BLACK);

	settextstyle(32, 0, L"system");
	outtextxy(20, 20, L"��Ļ������бʵʱ��������");
	settextstyle(28, 0, L"system");
	outtextxy(20, 60, L"Real-time tilt correction tool       by huidong 2021.10.16");

	settextstyle(14, 0, L"����");
	outtextxy(30, 430, L"������󣺣�������û����ȥ��ӰԺ���ҡ������� TC ��Ӱ��ʱ��¼���ߵ�����Ƕ���б�ˣ�");
	outtextxy(30, 454, L"���ƴ˹���ʵʱ������Ļ��б���档");

	RECT btn = { 200, 200, 420, 250 };
	setlinecolor(BLACK);
	rectangle(btn.left, btn.top, btn.right, btn.bottom);
	drawtext(L"ʵʱ���� >>", &btn, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

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
 * @brief ��ȡͼ����ָ����󳤿�ʱ�����ű���
 * @param img ͼ��ָ��
 * @param lw ͼ��������
 * @param lh ͼ������߶�
 * @return ����ͼ������ű���
*/
double GetImageRatioInSize(IMAGE* img, int lw, int lh)
{
	double ratio1 = (double)lw / img->getwidth();
	double ratio2 = (double)lh / img->getheight();
	return ratio1 < ratio2 ? ratio1 : ratio2;
}

// �Ե�Ϊ�����������
RECT MakeRectFromPoint(int x, int y, int radius)
{
	return { x - radius,y - radius,x + radius,y + radius };
}

// �ػ����
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

// �������ڴ�С
void ResizeWindow(int w, int h)
{
	closegraph();
	initgraph(w, h);
	setbkcolor(WHITE);
	cleardevice();
}

// ѡ����ת���򣬷��ش������λ�ú�ͼ��
AreaImage SelectRect(IMAGE* img)
{
	// �豸���
	int dw = 1080;
	int dh = 420;

	// ͼ��������ÿ��
	int imgarea_w = dw;
	int imgarea_h = dh - 60;

	// ͼ�����ƫ��
	int img_x = 0, img_y = 24;

	// ͼ����ʾ����
	RECT rctImageArea = { img_x,img_y,img_x + imgarea_w ,img_y + imgarea_h };

	// ����
	ResizeWindow(dw, dh);

	IMAGE imgResize = *img;

	// �������ű���
	double ratio = GetImageRatioInSize(img, rctImageArea.right - rctImageArea.left, rctImageArea.bottom - rctImageArea.top);
	ImageToSize((int)(img->getwidth() * ratio), (int)(img->getheight() * ratio), &imgResize);

	settextcolor(BLACK);
	settextstyle(24, 0, L"����");
	outtextxy(0, 0, L"���϶����Σ�ѡ����Ҫʵʱ����������");

	// ��ť
	RECT btn = { dw - 140, dh - 30, dw - 3, dh - 3 };
	setlinecolor(BLACK);
	settextstyle(16, 0, L"����");
	rectangle(btn.left, btn.top, btn.right, btn.bottom);
	drawtext(L"��һ�� >>", &btn, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

	// �û����õ�����
	RECT rct = { 100,100,200,200 };

	setlinecolor(LIGHTGREEN);
	setfillcolor(YELLOW);

	const int nRectRadius = 5;

	DrawRect(img_x, img_y, &imgResize, rct, nRectRadius, imgarea_w, imgarea_h);

	// �û�����
	ExMessage msg;
	int nPressId = -1;
	while (true)
	{
		// ���ε���������
		RECT vertex[2];
		vertex[0] = MakeRectFromPoint(rct.left, rct.top, nRectRadius);
		vertex[1] = MakeRectFromPoint(rct.right, rct.bottom, nRectRadius);

		// ��Ӧ�����϶�����ť���
		if (peekmessage(&msg, EM_MOUSE))
		{
			// ����ֻ����ͼ�������ڵ���
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

	// ���ݱ�����ԭ��������

	// ˳�����
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

	// �ü�������ͼ��
	SetWorkingImage(img);
	IMAGE cut;
	getimage(&cut, rct.left, rct.top, rw, rh);
	SetWorkingImage();

	return { cut,rct };
}

// ������б��
double SetRadian(IMAGE* img)
{
	ResizeWindow(640, 480);

	settextstyle(24, 0, L"����");
	settextcolor(BLACK);
	outtextxy(0, 0, L"�����ô��������б��");

	RECT btn[3] = { { 210, 420, 240, 450 },
					{ 310, 420, 340, 450 },
					{ getwidth() - 200, getheight() - 50, getwidth() - 3, getheight() - 3 } };

	setlinecolor(BLACK);
	for (int i = 0; i < 3; i++)
	{
		fillrectangle(btn[i].left, btn[i].top, btn[i].right, btn[i].bottom);
	}

	settextstyle(12, 0, L"����");
	drawtext(L"��", &btn[0], DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	drawtext(L"��", &btn[1], DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	drawtext(L"��ʼʵʱ���� >>", &btn[2], DT_CENTER | DT_SINGLELINE | DT_VCENTER);

	BeginBatchDraw();

	double radian = 0;
	int nPressId = -1;
	while (true)
	{
		// �ػ�
		clearrectangle(0, 24, getwidth(), 419);

		IMAGE rotated;
		rotateimage(&rotated, img, radian, WHITE, true);

		double ratio = GetImageRatioInSize(&rotated, getwidth(), getheight() - 100);
		ImageToSize((int)(rotated.getwidth() * ratio), (int)(rotated.getheight() * ratio), &rotated);
		putimage(0, 30, &rotated);

		FlushBatchDraw();

		// ��Ϣ��Ӧ
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
					// ���٣���ֹ��������
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

// ��ʼ��һ��ȫ������
void InitFullScreenWindow()
{
	initgraph(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	SetWindowLong(GetHWnd(), GWL_STYLE, GetWindowLong(GetHWnd(), GWL_STYLE) & ~WS_CAPTION);
	SetWindowPos(GetHWnd(), HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE);
}

// ��б����
void TiltCorrect(RECT rct, double radian)
{
	int w1 = rct.right - rct.left;
	int h1 = rct.bottom - rct.top;

	// ��ȡ��ת��ͼ���С
	IMAGE temp[2];
	temp[1].Resize(w1, h1);
	rotateimage(&temp[0], &temp[1], radian, WHITE, true);
	int w2 = temp[0].getwidth();
	int h2 = temp[0].getheight();

	const int nAddHeight = 40;

	// ���贰�ڴ�С
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
		// ȫ���ػ�
		if (bRedrawAll)
		{
			cleardevice();
			bRedrawControl = true;
			bRedrawAll = false;
		}

		// ͼ�����
		{
			// ��ͼ
			IMAGE img, imgRect, imgRotate;
			CaptureDesktop(&img);

			// �ü�
			SetWorkingImage(&img);
			getimage(&imgRect, rct.left, rct.top, w1, h1);
			SetWorkingImage();

			// ��ת������ģʽ��ʹ�õ�������ת��
			rotateimage(&imgRotate, &imgRect, radian, WHITE, true, !isFastMode);

			// ������������м���
			if ((int)(zoom * 10) != 10)
			{
				ImageToSize((int)(imgRotate.getwidth() * zoom), (int)(imgRotate.getheight() * zoom), &imgRotate);
			}

			// ����
			putimage(offset_x, offset_y, &imgRotate);
		}

		// �ػ�ؼ�
		if (bRedrawControl)
		{
			setlinecolor(BLACK);
			settextstyle(14, 0, L"����");
			settextcolor(BLACK);

			for (int i = 0; i < 2; i++)
			{
				fillrectangle(btn[i].left, btn[i].top, btn[i].right, btn[i].bottom);
			}

			outtextxy(10, h2 + 5, L"�����϶�ͼ�񣬹��ֲ���ͼ������");
			drawtext(L"ȫ����ESC �˳���", &btn[0], DT_CENTER | DT_SINGLELINE | DT_VCENTER);

			if (isFastMode)
			{
				drawtext(L"�л�����ģʽ", &btn[1], DT_CENTER | DT_SINGLELINE | DT_VCENTER);
			}
			else
			{
				drawtext(L"��������ģʽ", &btn[1], DT_CENTER | DT_SINGLELINE | DT_VCENTER);
			}

			line(0, h2 + 1, getwidth(), h2 + 1);

			bRedrawControl = false;
		}

		// ��Ϣ����
		if (peekmessage(&msg, EM_MOUSE))
		{
			// ȫ���²���Ӧ����
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
					bRedrawControl = true;	// �ػ�ؼ�
				}
			}

			// ͼ������
			if (msg.message == WM_MOUSEWHEEL)
			{
				zoom += msg.wheel * 0.0003;

				// ��������
				if (zoom < 0)
				{
					zoom = 0.05;
				}
				else if (zoom > 20)
				{
					zoom = 20;
				}
			}

			// ͼ���ƶ�
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
					if (!isFullWindow)
					{
						bRedrawAll = true;
					}
				}
			}
		}

		if (peekmessage(&msg, EM_KEY))
		{
			// �˳�ȫ��
			if (isFullWindow && msg.vkcode == VK_ESCAPE)
			{
				ResizeWindow(w2, h2 + nAddHeight);
				isFullWindow = false;
				bRedrawControl = true;	// �ػ�ؼ�
			}
		}

		Sleep(1);
	}

}

// ������
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