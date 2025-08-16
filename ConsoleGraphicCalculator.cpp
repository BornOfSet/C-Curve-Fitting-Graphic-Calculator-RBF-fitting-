// ConsoleGraphicCalculator.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include<glad/glad.h>
#include<glfw3.h>
#include<vector>
#include<Dense>


std::vector<float> _samples;
bool move_on = false;
double radius = 0.1;
float cursorx;
float cursory;

void w0_cursorpos_callback(GLFWwindow* window, double xpos, double ypos){
	
}

void w0_mousebutton_callback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_RELEASE)return;

	//这是为了把已经绘制东西的一面翻进去，我们只能绘制“底面”，所以为了绘制东西，需要把这面翻做“底面”
	glfwSwapBuffers(window);
	double x;
	double y;
	glfwGetCursorPos(window, &x, &y);

	//我们实际的绘图区域只有560
	//而且范围是20~580
	//如果没有这回事，那么直接除600就行了
	//如果这里的变换没有和实际glviewport对上，那么就会导致误差积累，越边缘缩放的越严重。因为0.9*1-1=-0.1, 0.9*100-100=-10
	//第一个/560应该能产生大于1的数字，因为x的范围是0~600
	//最终需要把0~1映射到20~580上
	x = (fmax(fmin(580, x), 20) - 20) / 560;
	y = (fmax(fmin(580, y), 20) - 20) / 560;
	y = 1 - y;
	x = x * 2 - 1;
	y = y * 2 - 1;
	_samples.push_back((float)x);
	_samples.push_back((float)y);

	//一个有趣的事实是，eventlistener和main是同一个线程，因此main里面如果设好了上下文，这里是能照样延续过来的
	//而如果两边的上下文不一样，那不是因为glfw做了什么黑箱黑箱里面有什么独立的新线程，而是因为我创建了w0 w1两个窗体
	//在w1创建后，把上下文改了
	//std::cout<<glfwGetCurrentContext()<<std::endl;
	//而......当然，你可以在glfw的回调里面直接唤起drawcall
	//没必要把它放进循环然后通过“绘制所依赖的状态的变化”来新增绘制内容
	glBegin(GL_POINTS);
	glColor3f(1, 1, 1);
	glVertex2d(x, y);
	glEnd();

	//旧的基础上画完东西再把它翻出来
	//显然，只能画不能删
	glfwSwapBuffers(window);
}


void w0_keypressed_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
		move_on = true;
	}
}


void w1_cursorpos_callback(GLFWwindow* window, double xpos, double ypos) {
	float x = xpos/600;
	float y = ypos/600;
	y = 1 - y;
	x = x * 2 - 1;
	y = y * 2 - 1;
	cursorx = x;
	cursory = y;
}


void w1_keypressed_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
		move_on = true;
	}
	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		radius += 0.0025;
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		radius -= 0.0025;
	}
}

//********************************************************************************************************************************************
//********************************************************************************************************************************************
//********************************************************************************************************************************************
//参考函数，你只应该改这一部分
float reffun(float x) {
	float y = x+cos(x);
	return y;
}
//********************************************************************************************************************************************
//********************************************************************************************************************************************
//********************************************************************************************************************************************


int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);



	//控制面板，移动并单击鼠标在上面放置点
	//按下Enter键确认全部放置
	//给定一个固定函数，用户可以调整遍历点的密度来绘制该函数
	//用户应该贴近拟合结果放置点
	GLFWwindow* w0 = glfwCreateWindow(600, 600, "Control", NULL, NULL);
	glfwSetCursorPosCallback(w0, w0_cursorpos_callback);
	glfwSetMouseButtonCallback(w0, w0_mousebutton_callback);
	glfwSetKeyCallback(w0, w0_keypressed_callback);
	glfwMakeContextCurrent(w0);
	//std::cout << glfwGetCurrentContext() << std::endl;
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glViewport(20, 20, 560, 560);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glPointSize(2);

	//遍历绘图
	std::cout << "调整视口以便得到合适的参考曲线" << std::endl;
	float scalar = 1;
	float scale = 1;
	float stride = 0.1;
	float offsetX = 0;
	float offsetY = 0;
	bool w0_exit = false;
	while (!w0_exit) {
		float xL = -1;
		glClear(GL_COLOR_BUFFER_BIT);
		//绘制坐标轴
		glLineWidth(2);
		glBegin(GL_LINES);
		glColor3f(1, 0, 0);
		glVertex2f(-1, 0);
		glVertex2f(1, 0);
		glVertex2f(0, -1);
		glVertex2f(0, 1);
		glEnd();
		glLineWidth(1);
		glBegin(GL_LINE_STRIP);
		while (xL <= 1) {
			float x = xL * scalar - offsetX;
			float y = reffun(x);
			glVertex2f(xL, (1 / scale) * y + offsetY);
			xL += abs(stride);
		}
		glEnd();
		glfwSwapBuffers(w0);
		std::cout << "满意？(1/0)" << std::endl;
		std::cin >> w0_exit;
		if (!w0_exit) {
			std::cout << "定义域范围" << std::endl;
			std::cin >> scalar;
			std::cout << "步长间隔" << std::endl;
			std::cin >> stride;
			std::cout << "缩小倍数" << std::endl;
			std::cin >> scale;
			std::cout << "图像横移" << std::endl;
			std::cin >> offsetX;
			std::cout << "Y轴平移" << std::endl;
			std::cin >> offsetY;
			std::cout << "\n" << std::endl;
		}
	}
	bool _auto = false;
	std::cout << "自动平均采样？(1/0)" << std::endl;
	std::cin >> _auto;


	//自动平均采样？
	if (!_auto) {
		std::cout << "移动鼠标并单击以创建样本\n按下Enter以继续" << std::endl;
		while (!move_on)
		{
			//打点，同时等待我们按下Enter键
			glfwPollEvents();
		}
	}
	else {
		float auto_stride = 0.05;
		std::cout << "采样间隔步长？" << std::endl;
		std::cin >> auto_stride;
		glfwSwapBuffers(w0);
		glBegin(GL_POINTS);
		glColor3f(1, 1, 1);
		for (float x = -1.0f;x < 1.0f;x += abs(auto_stride)) {
			float tempx = x * scalar - offsetX;
			float y = reffun(tempx);
			y = (1 / scale) * y + offsetY;
			_samples.push_back(x);
			_samples.push_back(y);
			glVertex2d(x, y);
		}
		glEnd();
		glfwSwapBuffers(w0);
	}




	glfwSetCursorPosCallback(w0, NULL);
	glfwSetMouseButtonCallback(w0, NULL);
	glfwSetKeyCallback(w0, NULL);


	//显示面板，同时显示所有点
	GLFWwindow* w1 = glfwCreateWindow(600, 600, "Display", NULL, NULL);
	glfwSetKeyCallback(w1, w1_keypressed_callback);
	glfwSetCursorPosCallback(w1, w1_cursorpos_callback);
	glfwMakeContextCurrent(w1);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glViewport(0, 0, 600, 600);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glLineWidth(1);
	glBegin(GL_LINES);
	glColor3f(.5, .5, .5);
	glVertex2f(-1, 0);
	glVertex2f(1, 0);
	glVertex2f(0, -1);
	glVertex2f(0, 1);
	glEnd();
	glfwSwapBuffers(w1);

	int _never_use = 0;
	std::cout << "\n...按下任意数字键继续\n" << std::endl;
	while (std::cin >> _never_use) {
		//测试0~1范围
		std::cout << "\n选择合适的淡出范围\n使用UP和DOWN键调节\n按下Enter键以继续" << std::endl;
		move_on = false;
		float pi = 3.14159265358979323846;//别瞎define，会和其他文件冲突
		while (!move_on) {
			glClear(GL_COLOR_BUFFER_BIT);
			//绘制坐标轴
			glLineWidth(1);
			glBegin(GL_LINES);
			glColor3f(.5, .5, .5);
			glVertex2f(-1, 0);
			glVertex2f(1, 0);
			glVertex2f(0, -1);
			glVertex2f(0, 1);
			glEnd();

			//复制点
			glPointSize(2);
			glColor3f(1, 1, 1);
			for (int i = 0;i < _samples.size();i += 2) {
				glBegin(GL_POINTS);
				glVertex2f(_samples[i], _samples[i + 1]);
				glEnd();
			}

			//绘制鼠标位置
			glColor3f(.8, .8, 0);
			glBegin(GL_POINTS);
			glVertex2f(cursorx, cursory);
			glEnd();

			//绘制强度
			float angle = 0;
			float u = -1 / (2 * radius * radius);
			float len = sqrt(log(0.001) / (u));
			glBegin(GL_LINE_STRIP);
			while (angle <= 2 * pi) {
				angle += pi / 4;
				float Xcoord = len * cos(angle);
				float Ycoord = len * sin(angle);
				glVertex2f(cursorx + Xcoord, cursory + Ycoord);
			}
			glEnd();

			glfwSwapBuffers(w1);
			glfwPollEvents();
		}

		//计算每个样本的“颜色”
		//（法线当然也是一种颜色，因为一个表面上的两点随着坐标不同而可能拥有同样的蓝色，指同样的法线方向）
		//（不随着坐标一起变化的，就叫做颜色）
		//Vn=V1*f1(P)+V2*f2(P)+V3*f3(P)
		int samples_size = _samples.size() / 2;
		Eigen::MatrixXd matfx(samples_size, samples_size);
		Eigen::VectorXd maty(samples_size);
		Eigen::VectorXd matw(samples_size);
		for (int i = 0;i < samples_size;i++) {
			for (int j = 0;j < samples_size;j++) {
				//注意，这里是横坐标，而不是点距离
				//咳咳，这是废话。因为如果我们的样本是坐标--颜色，那么肯定不是计算sqrt(Pos^2+Col^2)作为径向距离啊]
				double _DIST = _samples[2 * i] - _samples[2 * j];
				matfx(i, j) = exp((-1.0 / 2.0 / radius / radius) * pow(_DIST, 2.0));//小 心 类 型 转 化 取 整 0
			}
			maty(i) = _samples[2 * i + 1];
		}

		//QR分解是一个求解线性问题的极好办法
		//求得每点权重。如果Radius改变，权重也会跟着改变
		//如果radius范围太小，那么w=y，就是这个点没有找到其他样本嘛，显而易见，这个点就只有它自己的纯贡献
		// 
		//权重可以分解为 w=a*y
		//按照它的作图，我认为这是一种用于add运算的叠加状态
		//https://blog.csdn.net/u011426016/article/details/127454266
		//最后求得的平均值等同于原始的y数据
		//然而一个点是很多其他点的混合，所以它不能直接就等于平均值
		matw = matfx.colPivHouseholderQr().solve(maty);

		/*
		std::cout << "\nFx\n" << matfx << std::endl;
		std::cout << "\nY\n" << maty << std::endl;
		std::cout << "\nW\n" << matw << std::endl;
		*/
		glLineWidth(1);
		glBegin(GL_LINES);
		glColor3f(0, 0, 1);
		for (int i = 0;i < samples_size;i++) {
			glVertex2f(_samples[2 * i], 0);
			glVertex2f(_samples[2 * i], matw[i]);
		}
		glEnd();

		float dX = -1;
		glLineWidth(1);
		glColor3f(1, 0, 0);
		glBegin(GL_LINE_STRIP);
		while (dX < 1) {
			float Val = 0;
			for (int i = 0;i < samples_size;i++) {
				//显然， dist不是算x^2+y^2的，因为如果是这样的算的话，我现在的y可是未知数。难道是(xd,0)吗？
				float _DIST = _samples[2 * i] - dX;
				float fx = exp((-1.0 / 2.0 / radius / radius) * pow(_DIST, 2.0));
				Val += fx * matw[i];
			}
			glVertex2f(dX, Val);
			dX += abs(0.01);
		}
		glEnd();
		glfwSwapBuffers(w1);

		std::cout << "\n\n拟合完成\n按下任意数字键重新设置淡出范围\n结束全部拟合请按任意字母键" << std::endl;
	}

	std::cout << "\n\n退出运行。关闭任意窗口以退出" << std::endl;
	while (!glfwWindowShouldClose(w1)&& !glfwWindowShouldClose(w0))
	{
		//保持界面不要退出，观察拟合效果
		glfwPollEvents();
	}

	glfwTerminate();
}