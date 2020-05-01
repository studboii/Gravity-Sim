
#include<GL/glew.h>
#include<glut/glut.h>
#include<gl/GLU.h>//this is apparantly built in. Even glew was built in. We techically only needed to set the preprocessor thingy for it. GL is our include and gl is the built-in shit from Visual Studio.
#include<iostream>
#include<string>
#include<math.h>

/*
Equations of Motion to implement:
	Total Time of Flight:
		t = (2*u*sin(theta))/g

	At a time interval t

	x = u*t*cos(theta)

	y = u*t*sin(theta) - 0.5*(g*t*t)

	TODO:

	Use That time in air and calculate x and y at each point in time //sorta done

	Map that location to sphere's co-ordinates and move that shit.//some small oofs associated with this.

*/

void* Currentfont; //saves the font as a void pointer

void ResetValues();


void SetFont(void* font)
{
	Currentfont = font;
}//allows you to reset the font

void DrawString(double x, double y, double z, const char* string)
{
	const char* c;
	glRasterPos3d(x, y, z);

	for (c = string; *c != '\0'; c++)
	{
		glColor3f(0.0, 1.0, 1.0);
		glutBitmapCharacter(Currentfont, *c);
	}
}//This function draws our string in the position we ask it too. This way it's abstracted and cleaner if we have to display multiple strings

void reshape(int width, int height)
{
	glViewport(0, 0, 750, 750);
	//glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluOrtho2D(-10, 10, -10, 10);
	//glMatrixMode(GL_MODELVIEW);
}




enum class AppStat
{
	UNKNOWN = -1, START_SCREEN = 3, MENU = 4, PRJMTN_INP_INIT_VELOCITY = 0, PRJMTN_INP_THETA = 1,
	PRJMTN_DISP = 5, PRJMTN_PLOT = 6, DROP_INP_HT = 2, DROP_DISP = 7, DROP_PLOT = 8, ABOUT_PAGE = 10

};//the input states get 0 - 2 so I can use the arrays and not duplicate code for everything here.

bool TakeInput = false;
//AppStat is an enum that we can use to tell our display mode to switch to a particular mode. 
/* Unknown we can use to do Error Handling
	START_SCREEN tells it to display the first initial start screen, MENU is similar
	PRJMTN = Projectile Motion
	INP = input mode
	the next part tells us what we're taking in input for. I'll rename it optimise once we have a calc function that tells us what inputs it needs and compute the equation for it.
	DISP = Show the animation
	DROP = Dropping an object
	ABOUT_PAGE = redirect to about page
	*/



AppStat DispStat = AppStat::UNKNOWN;


float values[3];//the values we need are stored in this array as floats

std::string inp[3];//we save the strings we want to flush in this. They're small numbers so this should work fine for our purposes. Saving it here allows us to implement backspace easily and error hnadling later on is also easier
double DropPos = 8.0, DropTime = 0.0;
double TotalDropTime = 0.0;
double DropTCalc(float);
double ToF = 0.0;
double uSinTh = 0.0, uCosTh = 0.0, TimeInAir = 0.0;//u*sin(theta) and u*cos(th) precalculated to optimise for speed.
double xProj, yProj;
double TimeOfFlight(float, float);
void DrawGrid();
int pauseState = 0; //0 is play 1 is pause

void KeyProc(unsigned char key, int x, int y)//This is function bound to the keystroke from my keyboard
{

	if (key == 'q')//for debug, remove in deploy
	{
		exit(0);

	}
	if (key == 'b')//Remove on deploy Use this to go back to menu incase you need to test something.
	{
		DispStat = AppStat::MENU;
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(-1, 1, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glutPostRedisplay();
	}
	if (TakeInput)//we process and take input here. common processing for each varaible
	{
		if (key == 13)
		{
			//enter key	
			TakeInput = false;
			glutPostRedisplay();
		}
		else if (key == 8)
		{
			int len = inp[(int)DispStat].length();
			if (len - 1 <= 0)
			{
				values[(int)DispStat] = 0;
				inp[(int)DispStat] = "";
				glutPostRedisplay();
				return;
			}//if they enter backspace and there's nothing to clear.
			std::cout << len << std::endl;
			inp[(int)DispStat] = inp[(int)DispStat].substr(0, (len - 1));//extract everything but the last charcter here.
			std::cout << inp[(int)DispStat] << std::endl;
			values[(int)DispStat] = std::stof(inp[(int)DispStat]);//save the value so we can do stuff with it. I'll try and flush it into a buffer later on but there's no noticable performance impact as of now.
			glutPostRedisplay();
		}
		else
		{
			inp[(int)DispStat].push_back(key);
			values[(int)DispStat] = std::stof(inp[(int)DispStat]);
			glutPostRedisplay();
		}

	}
	if (DispStat == AppStat::START_SCREEN)
	{
		if (key == 'X' || key == 'x')
		{
			DispStat = AppStat::MENU; //now we're displaying the menu so we're changing the DispStat accordingly
			glutPostRedisplay();//we need to call our displayfunc again to render the next screen instead
		}
		else if (key == 'q' || key == 'Q')
		{
			exit(0);//user go bye byeeeee, khuda haifiz userji
		}
	}
	else if (DispStat == AppStat::MENU)
	{
		if (key == '1')
		{
			DispStat = AppStat::PRJMTN_INP_INIT_VELOCITY;
			TakeInput = true;
			glutPostRedisplay();
		}
		else if (key == '2')
		{
			DispStat = AppStat::DROP_INP_HT;
			TakeInput = true;
			glutPostRedisplay();
		}
		else if (key == '3')
		{
			DispStat = AppStat::ABOUT_PAGE;
			glutPostRedisplay();
		}
		else if (key == '4' || key == 'q' || key == 'Q')
		{
			exit(0);
		}
	}
	else if (DispStat == AppStat::PRJMTN_DISP || DispStat == AppStat::DROP_DISP)
	{
		if (key == 'p' || key == 'P' || key == ' ')
		{
			pauseState = pauseState == 0 ? 1 : 0;
			glutPostRedisplay();
		}
		else if (key == 'r' || key == 'R')
		{
			ResetValues();
			glutPostRedisplay();
			return;
		}
		else if (key == 's' || key == 'S')
		{
			ResetValues();
			if (DispStat == AppStat::PRJMTN_DISP)
				DispStat = AppStat::PRJMTN_PLOT;
			else if (DispStat == AppStat::DROP_DISP)
				DispStat = AppStat::DROP_PLOT;
			//move to next page for final values, if not leave it here idk TBD That's why else if and not just an else
		}
	}
}

void plotTrajectory();

void animater(int a)
{
	
	if (a == 1)
	{
		glutPostRedisplay();
		return;
	}
	if (DispStat == AppStat::DROP_DISP)
	{
		if (DropTime < TotalDropTime && DropPos > -47.5)
		{
			DropTime = DropTime + 0.016667;//this wasn't taking 1/60 for some reason and I hard-coded and made the rest doubles for it to work
			double DisInM = -4.9 * (DropTime * DropTime); // s = 0.5gt^2 => 0.5g = 4.9 (negative because we goin down.)
			std::cout << "\nSphere should've moved by:\n";
			std::cout << DisInM;
			std::cout << "\nTime Elapsed = \n";
			std::cout << DropTime;

			DropPos += DisInM;//remove distance travelled 




		}
	}
	else if (DispStat == AppStat::PRJMTN_DISP)
	{
		if (TimeInAir < ToF)
		{
			/*
				At a time interval t

					x = u*t*cos(theta)

					y = u*t*sin(theta) - 0.5*(g*t*t)
			*/
			xProj = (uCosTh * TimeInAir) - 87.7;//87.7 is leftmost co-ordinate
			yProj = uSinTh * TimeInAir - 4.9 * TimeInAir * TimeInAir;
			TimeInAir += 0.016666667;
		}
	}
	glutPostRedisplay();
}


void disp()
{
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);			// White Background
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);


	if (DispStat == AppStat::START_SCREEN)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		SetFont(GLUT_BITMAP_HELVETICA_18);
		glColor3f(0.0, 0.0, 0.0);
		DrawString(-0.3, 0.8, 0.0, "Projectile Motion and Garvity Simulation");

		DrawString(0.15, 0.7, 0.0, " -By Jawad and Gaurav");

		DrawString(-0.3, 0.0, 0.0, "Press X to Begin");
		DrawString(-0.3, -0.1, 0.0, "Press Q to Quit");
		//std::cout << (int)DispStat << std::endl;

		glFlush();
	}

	else if (DispStat == AppStat::MENU)
	{
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		ResetValues();

		SetFont(GLUT_BITMAP_HELVETICA_18);
		glColor3f(0.0, 0.0, 0.0);
		DrawString(-0.3, 0.8, 0.0, "Menu");
		DrawString(-0.3, 0.6, 0.0, "1) Projectile Simulation");
		DrawString(-0.3, 0.5, 0.0, "2) Dropping an Object");
		DrawString(-0.3, 0.4, 0.0, "3) About");
		DrawString(-0.3, 0.3, 0.0, "4) Exit (You can press Q to leave)");
		DrawString(-0.3, 0.1, 0.0, "Type in 1,2,3 or 4 Depending on your choice:");
		//std::cout << (int)DispStat << std::endl;
		glFlush();
	}

	else if (DispStat == AppStat::PRJMTN_INP_INIT_VELOCITY)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		SetFont(GLUT_BITMAP_HELVETICA_18);
		glColor3f(0.0, 0.0, 0.0);
		DrawString(-0.3, 0.8, 0.0, "Projectile Screen");
		if (TakeInput)
		{
			DrawString(-0.3, 0.5, 0.0, "Initial Velocity (m/s):");
			char buf[20];
			snprintf(buf, sizeof(buf), "%f", values[0]); //converts a float into a character array so we can display it
			DrawString(-0.3, 0.3, 0.0, buf);
		}
		else
		{
			DispStat = AppStat::PRJMTN_INP_THETA;
			TakeInput = true;
			glutSwapBuffers();
			glutPostRedisplay();

		}
		glFlush();
	}
	else if (DispStat == AppStat::PRJMTN_INP_THETA)
	{
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		SetFont(GLUT_BITMAP_HELVETICA_18);
		glColor3f(0.0, 0.0, 0.0);
		DrawString(-0.3, 0.8, 0.0, "Projectile Screen for the second value:");
		if (TakeInput)
		{
			DrawString(-0.3, 0.5, 0.0, "Projectile Angle:");
			char buf[20];
			snprintf(buf, sizeof(buf), "%f", values[1]);
			DrawString(-0.3, 0.3, 0.0, buf);

		}
		else
		{
			DispStat = AppStat::PRJMTN_DISP;


			values[1] = (4.0 * std::atan2(1.0, 1.0)) * values[1] / 180.0;//deg2Radians
			ToF = TimeOfFlight(values[0], values[1]);
			std::cout << "The Time of Flight is:\n";

			std::cout << ToF << std::endl;
			uSinTh = values[0] * sin(values[1]);
			uCosTh = values[0] * cos(values[1]); //Saving u*sin(theta) and u*cos(theta) so we won't waste compute time with calls

			glutPostRedisplay();

		}
		glFlush();
	}

	else if (DispStat == AppStat::PRJMTN_DISP)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		glMatrixMode(GL_PROJECTION);
		//call animater once every 1000/60th of a milli second. (60fps is the refresh atm)
		glLoadIdentity();
		gluOrtho2D(-100, 100, -100, 100);
		//I am changing the projection to 100 total span. we can make it bigger but This looks fine so far.
		glMatrixMode(GL_MODELVIEW);
		//we basically move our camera by the distance specified here.
		//plotTrajectory(); --> Shows you the expected trajectory uncomment to verify that.

		DrawGrid();
		glColor3f(0, 1, 1);
		glutTimerFunc(1000/60, animater, pauseState);

		//Displaying Movement Options
		DrawString(-90, -20, 0, "P -> Play/Pause");
		DrawString(-90, -25, 0, "R -> Restart");
		DrawString(-90, -30, 0, "S -> Plot Trajectory");

		//Displaying Current Values
		DrawString(40, -20, 0, "Height: ");
		char bufHt[20];
		double ht;
		if (yProj <= 0.13)
			ht = 0.0;
		else
			ht = yProj;
		//It fixes the height issue where it's slightly higher on the bottom because of the last frame that gets skipped or something, I am not sure. It looks better this way.
		snprintf(bufHt, sizeof(bufHt), "%f", ht); //converts a float into a character array so we can display it
		DrawString(60, -20, 0, bufHt);

		char bufDis[20];
		snprintf(bufDis, sizeof(bufDis), "%f", (xProj + 87.7)); 
		DrawString(40, -25, 0, "Distance: ");
		DrawString(70, -25, 0, bufDis);

		char bufTime[20];
		snprintf(bufTime, sizeof(bufTime), "%f", TimeInAir);
		DrawString(40, -30, 0, "Time in Air: ");
		DrawString(70, -30, 0, bufTime);



		glTranslated(xProj, yProj, 0);
		//glTranslated(-95, 0, 0);//Right now it is at the leftmost part of the screen. Yet to be seen if I can travel 200m using it.
		glutSolidSphere(2.5, 100, 100);//Start form 0 and travels exactly to the grid at the moment. Math is correct we need to figure out how to modify mapping and then this will be done.


		glFlush();
	}

	else if (DispStat == AppStat::PRJMTN_PLOT)
	{
		//should we animate trajectory? TBD

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		glMatrixMode(GL_PROJECTION);
		//call animater once every 1000/60th of a milli second. (60fps is the refresh atm)
		glLoadIdentity();
		gluOrtho2D(-100, 100, -100, 100);
		//I am changing the projection to 100 total span. we can make it bigger but This looks fine so far.
		glMatrixMode(GL_MODELVIEW);
		plotTrajectory();// --> Shows you the expected trajectory

		DrawGrid();
		glColor3f(0, 1, 1);

	}

	else if (DispStat == AppStat::DROP_INP_HT)
	{
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		SetFont(GLUT_BITMAP_HELVETICA_18);
		glColor3f(0.0, 0.0, 0.0);
		DrawString(-0.3, 0.8, 0.0, "Drop Motion Simulator");
		if (TakeInput)
		{
			DrawString(-0.3, 0.5, 0.0, "Enter the height of the drop:");
			char buf[20];
			snprintf(buf, sizeof(buf), "%f", values[2]);//converts a float into a character array so we can display it
			DrawString(-0.3, 0.3, 0.0, buf);

		}
		else
		{
			DispStat = AppStat::DROP_DISP;
			//Debug stuff is here
			/*std::cout << "\nThe value for height entered is:\n";
			std::cout << values[2];*/

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluOrtho2D(-50, 50, -50, 50);//
			glMatrixMode(GL_MODELVIEW);
			DropPos = values[2] - 50 - 2.5;//Initialising where you drop or drop height.
			//glutTimerFunc(0, animater, 0);
			TotalDropTime = DropTCalc(values[2]);
			glutPostRedisplay();

		}
		glFlush();

	}

	else if (DispStat == AppStat::DROP_DISP)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glLoadIdentity();

		glMatrixMode(GL_PROJECTION);
		glutTimerFunc(1000 / 60, animater, pauseState);//call animater once every 1000/60th of a milli second. (60fps is the refresh atm)
		glLoadIdentity();
		gluOrtho2D(-50, 50, -50, 50);
		//I am changing the projection to 100 total span. we can make it bigger but This looks fine so far.
		glMatrixMode(GL_MODELVIEW);
		glTranslatef(15, DropPos, 0);//we basically move our camera by the distance specified here.

		/*We need to do this because we can only draw a sphere at the origin. So, I change where the origin is,
		instead of moving object. 60 looks smooth and because we have a black~ish background, no dropped frames
		that I can make out. */



		glutSolidSphere(2.5, 100, 100);//draws a solid sphere of radius 2.5. This is thus far, the most computationally intensive part from what I could gather. I don't have a GPU I can loan this too nor am I using shader. So this will be it.

		glFlush();
	}




	else if (DispStat == AppStat::ABOUT_PAGE)
	{

		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glLoadIdentity();

		SetFont(GLUT_BITMAP_HELVETICA_18);
		glColor3f(0.0, 0.0, 0.0);
		DrawString(-0.3, 0.8, 0.0, "About:");
		glFlush();
	}
	glutSwapBuffers();


}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);//initialising glut
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(10, 10);//initial position in pixels This is optional and not specifying it will mean window is at random location.
	glutInitWindowSize(1000,1000);//size of the window
	glutCreateWindow("TESTING WINDOW");
	glutDisplayFunc(disp);
	glutReshapeFunc(reshape);
	
	DispStat = AppStat::START_SCREEN;


	glutKeyboardFunc(KeyProc);


	glutMainLoop();
	return 0;


}

double DropTCalc(float dis)
{
	double temp = (2 * dis) / 9.8;
	double tm = sqrt(temp);
	return tm;
}

double TimeOfFlight(float velo, float ang)
{
	/*
		t = (2*u*sin(theta))/g
	*/
	double t = 2 * velo * sin(ang) / 9.8;
	return t;


}

void ResetValues()
{
	//modify to display full value set later on, maybe pass an enum and evaluate or use PolyMorphism.
	values[0] = 0;
	values[1] = 0;
	values[2] = 0;
	inp[0] = "";
	inp[1] = "";
	inp[2] = "";
	xProj = -87.7;//decent start? It looks right here.
	yProj = 0.0;
	TimeInAir = 0.0;
}

void plotTrajectory()
{
	double i = 0;

	//double xProj, yProj;
	for ( i = 0, xProj = 0, yProj = 0; i < ToF; i+= 0.0166667)
	{
		glPointSize(3);
		 xProj = uCosTh * i - 87.7;
		 yProj = uSinTh * i - 4.9 * i * i;
		glBegin(GL_POINTS);
		
		//TimeInAir += 0.016666667;
		glVertex2d(xProj, yProj);
		glEnd();
		glFlush();
	}

	std::cout << "i after flush = " << i << std::endl;
	std::cout << "Range covered = " << (xProj+87.7) << std::endl;
}

void DrawGrid()
{
	glBegin(GL_LINES);
		glColor3f(1, 0, 0);
		//X-Axis (ground)
		glVertex2f(-90, -2.6);//screen left
		glVertex2f(90, -2.6);//screen right

		//Y-Axis
		glVertex2f(-90, -2.6);//screen bottom
		glVertex2f(-90, 100);//screen top

	glEnd();

}