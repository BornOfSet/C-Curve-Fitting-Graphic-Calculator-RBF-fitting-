
#include <iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<Eigen/Dense>
#include<glm/vec2.hpp>
#include<glm/vec3.hpp>
#include<vector>
#include<glm/geometric.hpp>

using namespace std;
using namespace glm;

struct point {
    vec2 pos;//基础坐标系，世界空间。它是基础的，因为所有插值过程都是以它作为自变量的
    vec3 col;//不仅可以储存颜色，也可以储存样本的方向（参考perlin noise）
    vec2 dir;
};

point p_queried;
vec2 rad;
vec2 col_handle;
point* entity = nullptr;
vector<point> p_samples;
vec3 col_preview = vec3(1,1,1);

bool f_cDown;
bool f_bDown;
bool f_lmb;

float chroma_inversesqrt(float x) {
    return x == 0 ? 0 : (1.0f / sqrt(x));
}


point* FindNearestPoint(vec2 self) {
    float min_dist = 2;
    point* found = nullptr;
    //引用传递。如果是point p : p_samples就变成了值传递，结果每个值都是局部的，储存在同一个地方以便循环反复覆写，那就不对了。我们要引用实际地址
    for (point &p : p_samples) {
        float dist = distance(self , p.pos);
        if (min_dist > dist) {
            min_dist = dist;
            found = &p;
        }
    }
    if (min_dist <= 0.018) {
        return found;
    }
    else {
        return nullptr;
    }
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    switch (action) {
    case GLFW_PRESS:
        switch (key) {
        case GLFW_KEY_C:
            f_cDown = true;
            entity = FindNearestPoint(p_queried.pos);
            if (!entity)cout << "Not Found" << endl;
            break;
        case GLFW_KEY_B:
            rad = vec2(0, 0);
            f_bDown = true;
            break;
        }
        break;
    case GLFW_RELEASE:
        f_cDown = false;
        f_bDown = false;
        glfwSetCursorPos(window , p_queried.pos.x * 300 + 300, 300 - p_queried.pos.y * 300);//忽略拉开的半径，恢复位置
        entity = nullptr;
        col_handle = vec2(0);
        //cout << "高斯半径：" << length(rad) << endl;

    }
}

void MouseClickCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_1 && !f_cDown && !f_bDown) {
        entity = FindNearestPoint(p_queried.pos);
        if (entity) {
            f_lmb = true;
        }
        else {
            double xp, yp;
            glfwGetCursorPos(window, &xp, &yp);
            xp = xp / 600;
            yp = yp / 600;
            xp = xp * 2 - 1;
            yp = yp * 2 - 1;
            yp = -yp;
            point pos = { vec2(xp,yp),vec3(1,1,1),vec2(0,0) };
            p_samples.push_back(pos);
        }
    }
    if (action == GLFW_RELEASE) {
        f_lmb = false;
    }
}

void CursorRenCallback(GLFWwindow* window, double xpos, double ypos) {
    vec2 cursor = vec2(xpos / 300 - 1, -ypos / 300 + 1);
    if (f_bDown) {
        rad = cursor - p_queried.pos;
    }
    else if (f_cDown) {
        if (entity) {
            vec2 range = cursor - p_queried.pos;
            col_handle = range;
            float pertentage = length(range);
            float r = fmax(length(rad),0.001);//防止全黑，找不到点
            pertentage = fmin(r, pertentage)/ r;
            //为了这个色度算法在range长度为0时输出蓝色，我手动计算了normalize
            //使得inversqrt优化了特殊情况，如果分母为0，它会输出0
            //这样，0向量对应全蓝
            range = range * chroma_inversesqrt(dot(range, range));
            //色度
            float chromacity_r = abs(range.x) * pertentage;
            float chromacity_g = abs(range.y) * pertentage;
            //不在这里恢复成颜色。仍然保持色度
            col_preview = vec3(chromacity_r, chromacity_g, 1 - chromacity_g - chromacity_r);
            //col_preview = min(col_preview, vec3(1, 1, 1));
            entity->col = col_preview;
        }
    }
    else if (f_lmb) {
        vec2 range = cursor - p_queried.pos;
        range = range / length(range);
        entity->dir = range*vec2(0.18);
    }
    else {
        p_queried.pos = cursor;
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* win = glfwCreateWindow(600, 600, "PlanarFit", NULL, NULL);
    glfwSetMouseButtonCallback(win, MouseClickCallback);
    glfwSetCursorPosCallback(win, CursorRenCallback);
    glfwSetKeyCallback(win, KeyCallback);
    glfwMakeContextCurrent(win);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        //如果初始化失败，会引发异常 0xC0000005: 读取位置 0x000000000000001C 时发生访问冲突。
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    cout << "在平面上单击空白处并放置点。\n鼠标在屏幕内任意位置上时，按下B键，拖动鼠标调整范围。该范围将用作调整颜色的参考，也是高斯范围\n" << endl;

    cout << "鼠标悬浮在一个点上，按下C键，然后拉动鼠标，可以调整它的色相。\n距离原点越近越蓝。\n超出范围将不拥有任何蓝色。\n\n单击一个已经存在的点，将不放置新点，而是拖动鼠标可以改变它的朝向。\n在平面上拖动鼠标将根据已有点进行色彩估计、朝向估算、位置拟合。"  << endl;

    //就算没有定义，NDC也不会映射到零向量上
    //glViewport(0, 0, 600, 600);

    p_queried.col = vec3(0, 1, 1);
    p_queried.dir = vec2(0, 0);



    while (!glfwWindowShouldClose(win))//glfwWindowShouldClose函数在每次循环开始前检查一次GLFW是否被要求退出，如果是的话该函数返回true，渲染循环结束。
    {
        //渲染指令
        glClearColor(0,0,0,0);//清除颜色设置为X色
        glClear(GL_COLOR_BUFFER_BIT);//执行清除


        glLineWidth(36);
        glBegin(GL_LINES);
        glColor3f(col_preview.x*3, col_preview.y*3, col_preview.z*3);
        glVertex2f(-2, -1);
        glVertex2f(2, -1);
        glEnd();



        glPointSize(4);
        glLineWidth(1);
        //绘制样本方向
        glBegin(GL_LINES);
        for (point p : p_samples) {
            glColor3f(p.col.x, p.col.y, p.col.z);
            glVertex2f(p.pos.x, p.pos.y);
            glVertex2f(p.pos.x + p.dir.x, p.pos.y + p.dir.y);
        }
        //绘制鼠标方向
        glVertex2f(p_queried.pos.x, p_queried.pos.y);
        glVertex2f(p_queried.pos.x + p_queried.dir.x, p_queried.pos.y + p_queried.dir.y);
        glEnd();

        vec2 average_pos = vec2(0);
        glBegin(GL_POINTS);
        //绘制样本
        for (point p : p_samples) {
            //从色度重建RGB
            glColor3f(p.col.x*3, p.col.y*3, p.col.z*3);
            glVertex2f(p.pos.x, p.pos.y);
            average_pos += p.pos;
        }
        //得到平均中心
        average_pos /= p_samples.size();

        //绘制鼠标位置
        glColor3f(p_queried.col.x, p_queried.col.y, p_queried.col.z);
        glVertex2f(p_queried.pos.x, p_queried.pos.y);
        glEnd();


        //绘制radius end
        glBegin(GL_LINES);
        glColor3f(.5, .5, 0);
        glVertex2f(p_queried.pos.x, p_queried.pos.y);
        glVertex2f(p_queried.pos.x + rad.x, p_queried.pos.y + rad.y);
        glVertex2f(p_queried.pos.x, p_queried.pos.y);
        glVertex2f(p_queried.pos.x + col_handle.x, p_queried.pos.y + col_handle.y);
        glEnd();


        //操纵杆
        if(f_cDown){
            glBegin(GL_LINES);
            glColor3f(0, .2, 0);
            glVertex2f(p_queried.pos.x, p_queried.pos.y);
            glVertex2f(p_queried.pos.x + .1, p_queried.pos.y + 0);
            glVertex2f(p_queried.pos.x, p_queried.pos.y);
            glVertex2f(p_queried.pos.x + 0, p_queried.pos.y + .1);
            glVertex2f(p_queried.pos.x, p_queried.pos.y);
            glVertex2f(p_queried.pos.x + 0, p_queried.pos.y - .1);
            glVertex2f(p_queried.pos.x, p_queried.pos.y);
            glVertex2f(p_queried.pos.x - .1, p_queried.pos.y + 0);
            glEnd();
        }

        //计算基础方向以便更可靠的方向拟合
        //以平均点为球心
        //看我们的鼠标在簇的哪个方向上
        vec2 base_dir = p_queried.pos - average_pos;
        base_dir = base_dir / length(base_dir);

        int s = p_samples.size();
        //拟合当前位置
        if (s >= 2) {
            float r = length(rad);
            Eigen::MatrixXd matfx(s, s);
            Eigen::MatrixXd matYpos(s, 2);
            Eigen::MatrixXd matYcol(s, 3);
            Eigen::MatrixXd matYdir(s, 2);

            Eigen::MatrixXd matWpos(s, 2);
            Eigen::MatrixXd matWcol(s, 3);
            Eigen::MatrixXd matWdir(s, 2);

            for (int i = 0;i < s;i++) {
                for (int j = 0;j < s;j++) {
                    //求每两个样本之间的影响关系，以便得出一个让落在样本上的点拟合在样本上的矩阵
                    double _DIST = length(p_samples[i].pos - p_samples[j].pos);
                    matfx(i, j) = exp((-1.0 / 2.0 / r / r) * pow(_DIST, 2.0));
                }
                //它是距离对某种任一属性的映射
                //这种映射关系使得我们可以根据距离来估计所需的属性
                //它使得颜色可以是距离的函数，并且估算一个新的颜色
                //它可以这样拟合，是因为计算的就是概率密度分布

                //标准化的坐标，使得零影响映射自身
                //RBF的主要问题是，让取0的地方不要变成0.让不应该取0的地方不要取0
                matYpos(i, 0) = p_samples[i].pos.x - p_queried.pos.x;
                matYpos(i, 1) = p_samples[i].pos.y - p_queried.pos.y;

                matYcol(i, 0) = p_samples[i].col.x;
                matYcol(i, 1) = p_samples[i].col.y;
                matYcol(i, 2) = p_samples[i].col.z;

                //有一个优化思路，给出基础方向，把方向视作偏转。
                //注意，base_dir是要加到p_samples.dir上算偏转的（四边形法则就是偏转）
                //所以，一定要记得，保持它们模长一样啊，不然就 第一次压扁成这样的我
                //vec2 deviant = p_samples[i].dir / length(p_samples[i].dir) - base_dir;
                //matYdir(i, 0) = deviant.x / length(deviant);
                //matYdir(i, 1) = deviant.y / length(deviant);

                //之前写的有点问题。现在先找到空间内实际的末端，然后计算偏离中心多少，再用和pos相同的逻辑算出来新的末端
                //我觉得，如果半径太长，会有问题。导致其他点的贡献不是接近0，所以它带来的影响会变得剧烈
                matYdir(i, 0) = p_samples[i].dir.x + p_samples[i].pos.x - p_queried.pos.x;
                matYdir(i, 1) = p_samples[i].dir.y + p_samples[i].pos.y - p_queried.pos.y;

            }
            auto A = matfx.fullPivLu();
            matWcol = A.solve(matYcol);
            matWdir = A.solve(matYdir);
            matWpos = A.solve(matYpos);

            vec2 pos = vec2(0);
            vec3 col = vec3(0);
            vec2 dir = vec2(0);
            for (int i = 0;i < s;i++) {
                float _DIST = length(p_samples[i].pos - p_queried.pos);
                float fx = exp((-1.0 / 2.0 / r / r) * pow(_DIST, 2.0));
                //发现了个小问题，这样拟合位置会使得距离远的点跑到原点上
                pos += vec2(fx * matWpos(i, 0), fx * matWpos(i, 1));
                //距离远的点，fx接近0，所以颜色变成黑色
                col += vec3(fx * matWcol(i, 0), fx * matWcol(i, 1), fx * matWcol(i, 2));
                dir += vec2(fx * matWdir(i, 0), fx * matWdir(i, 1));
            }
            //添加了p_queried.pos，使得在前半部分加数为0时，得到的是p_queried.pos
            //嗯，这部分的表现应该合格了
            //剩下的就是颜色和法线，你最好不要在后期对它们进行调整，而是一开始就设好合适的y
            //不然其实行为是很难预测的，不够合理的拟合
            pos += p_queried.pos;
            //为颜色计算色度，避免在空白处被算成0
            //需要未被压缩的col_preview
            //需要保持线性关系，才能保证重建色度一致
            //不大对，这会导致花里斑斓的
            //col = col / vec3(col.r + col.g + col.b);
            col *= 3;
            //三个量都规范化了，可以拟合空间中任意一点
            //dir = dir / length(dir) * vec2(0.18);

            //更好地规范化方法是重设中心（RBF需要关心零值）
            //dir /= s;
            //dir += base_dir;
            //dir = dir / length(dir)* vec2(0.18);

            dir += p_queried.pos;

            glPointSize(6);
            glBegin(GL_POINTS);
            glColor3f(col.x, col.y, col.z);
            glVertex2f(pos.x, pos.y);
            glEnd();
            glBegin(GL_LINES);
            glVertex2f(pos.x, pos.y);
            glVertex2f(pos.x + dir.x, pos.y + dir.y);
            glEnd();
        }
        

        //检查并调用事件，交换缓冲
        glfwSwapBuffers(win);//glfwSwapBuffers函数会交换颜色缓冲（储存着GLFW窗口每一个像素颜色值的大缓冲），它在这一迭代中被用来绘制，并且将会作为输出显示在屏幕上。
        glfwPollEvents();//glfwPollEvents函数检查是否触发事件（比如键盘输入、鼠标移动）、更新窗口状态，并调用对应地回调函数（可以通过回调方法手动设置）
    }
    return 0;
}
