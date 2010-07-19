/****************************************************************************
 *
 * $Id$
 *
 * Copyright (C) 1998-2010 Inria. All rights reserved.
 *
 * This software was developed at:
 * IRISA/INRIA Rennes
 * Projet Lagadic
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * http://www.irisa.fr/lagadic
 *
 * This file is part of the ViSP toolkit
 *
 * This file may be distributed under the terms of the Q Public License
 * as defined by Trolltech AS of Norway and appearing in the file
 * LICENSE included in the packaging of this file.
 *
 * Licensees holding valid ViSP Professional Edition licenses may
 * use this file in accordance with the ViSP Commercial License
 * Agreement provided with the Software.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Contact visp@irisa.fr if any conditions of this licensing are
 * not clear to you.
 *
 * Description:
 * Example of model based tracking.
 *
 * Authors:
 * Nicolas Melchior
 * Romain Tallonneau
 *
 *****************************************************************************/

/*!
  \file mbtTracking.cpp

  \brief Example of dot tracking on an image sequence containing a cube.
*/

#include <visp/vpConfig.h>
#include <visp/vpDebug.h>

#include <visp/vpImageIo.h>
#include <visp/vpIoTools.h>
#include <visp/vpMath.h>
#include <visp/vpHomogeneousMatrix.h>
#include <visp/vpDisplayGTK.h>
#include <visp/vpDisplayX.h>
#include <visp/vpDisplayGDI.h>

#include <visp/vpMbtTracker.h>

#include <visp/vpVideoReader.h>
#include <visp/vpParseArgv.h>

#if (defined VISP_HAVE_XML2)

#define GETOPTARGS  "x:m:i:n:dcht"


void usage(const char *name, const char *badparam)
{
  fprintf(stdout, "\n\
Example of tracking based on the 3D model.\n\
\n\
SYNOPSIS\n\
  %s [-i <test image path>] [-x <config file>]\n\
  [-m <model name>] [-n <initialisation file base name>]\n\
  [-t] [-c] [-d] [-h]",
  name );

  fprintf(stdout, "\n\
OPTIONS:                                               \n\
  -i <input image path>                                \n\
     Set image input path.\n\
     From this path read images \n\
     \"ViSP-images/mbt/cube/image.%%04d.ppm\". These \n\
     images come from ViSP-images-x.y.z.tar.gz available \n\
     on the ViSP website.\n\
     Setting the VISP_INPUT_IMAGE_PATH environment\n\
     variable produces the same behaviour than using\n\
     this option.\n\
\n\
  -x <config file>                                     \n\
     Set the config file (the xml file) to use.\n\
     The config file is used to specify the parameters of the tracker.\n\
\n\
  -m <model name>                                 \n\
     Specify the name of the file of the model\n\
     The model can either be a vrml model or a .cao file.\n\
\n\
  -n <initialisation file base name>                                            \n\
     Base name of the initialisation file. The file will be 'base_name'.init .\n\
     This base name is also used for the optionnal picture specifying where to \
     click (a .ppm picture),\
\n\
  -t \n\
     Turn off the display of the the moving edges. \n\
\n\
  -d \n\
     Turn off the display.\n\
\n\
  -c\n\
     Disable the mouse click. Usefull to automaze the \n\
     execution of this program without humain intervention.\n\
\n\
  -h \n\
     Print the help.\n\n");

  if (badparam)
    fprintf(stdout, "\nERROR: Bad parameter [%s]\n", badparam);
}


bool getOptions(int argc, const char **argv, std::string &ipath, std::string &configFile, std::string &modelFile, std::string &initFile, bool &displayMovingEdge, bool &click_allowed, bool &display)
{
  const char *optarg;
  int   c;
  while ((c = vpParseArgv::parse(argc, argv, GETOPTARGS, &optarg)) > 1) {

    switch (c) {
    case 'i': ipath = optarg; break;
    case 'x': configFile = optarg; break;
    case 'm': modelFile = optarg; break;
    case 'n': initFile = optarg; break;
    case 't': displayMovingEdge = false; break;
    case 'c': click_allowed = false; break;
    case 'd': display = false; break;
    case 'h': usage(argv[0], NULL); return false; break;

    default:
      usage(argv[0], optarg);
      return false; break;
    }
  }

  if ((c == 1) || (c == -1)) {
    // standalone param or error
    usage(argv[0], NULL);
    std::cerr << "ERROR: " << std::endl;
    std::cerr << "  Bad argument " << optarg << std::endl << std::endl;
    return false;
  }

  return true;
}

int
main(int argc, const char ** argv)
{
  std::string env_ipath;
  std::string opt_ipath;
  std::string ipath;
  std::string opt_configFile;
  std::string configFile;
  std::string opt_modelFile;
  std::string modelFile;
  std::string opt_initFile;
  std::string initFile;
  bool displayMovingEdge = true;
  bool opt_click_allowed = true;
  bool opt_display = true;
  
  // Get the VISP_IMAGE_PATH environment variable value
  char *ptenv = getenv("VISP_INPUT_IMAGE_PATH");
  if (ptenv != NULL)
    env_ipath = ptenv;

  // Set the default input path
  if (! env_ipath.empty())
    ipath = env_ipath;


  // Read the command line options
  if (getOptions(argc, argv, opt_ipath, configFile, modelFile, initFile, displayMovingEdge, opt_click_allowed, opt_display) == false) {
    exit (-1);
  }

  // Get the option values
  if (!opt_ipath.empty())
    ipath = opt_ipath;
  else
    ipath = env_ipath + vpIoTools::path("/ViSP-images/mbt/cube/image%04d.pgm");
  
  if (!opt_configFile.empty())
    configFile = opt_configFile;
  else
    configFile = env_ipath + vpIoTools::path("/ViSP-images/mbt/cube.xml");
  
  if (!opt_modelFile.empty())
    modelFile = opt_modelFile;
  else
    modelFile = env_ipath + vpIoTools::path("/ViSP-images/mbt/cube.wrl");
  
  if (!opt_initFile.empty())
    initFile = opt_initFile;
  else
    initFile = env_ipath + vpIoTools::path("/ViSP-images/mbt/cube");
  
  std::cout << ipath << std::endl;

  vpImage<unsigned char> I;
  vpVideoReader reader;

  reader.setFileName(ipath.c_str());
  reader.open(I);
  
  reader.acquire(I);

    // initialise a  display
#if defined VISP_HAVE_X11
    vpDisplayX display;
#elif defined VISP_HAVE_GDI
    vpDisplayGDI display;
#elif defined VISP_HAVE_D3D
    vpDisplayD3D display;
#endif
  if (opt_display)
  {
    display.init(I, 100, 100, "Test tracking") ;
    vpDisplay::display(I) ;
    vpDisplay::flush(I);
  }
 
  vpMbtTracker tracker;
  vpHomogeneousMatrix cMo;
  
  // Load tracker config file (camera parameters and moving edge settings)
  tracker.loadConfigFile(configFile.c_str());

  // Display the moving edges, see documentation for the significations of the colour
  tracker.setDisplayMovingEdges(displayMovingEdge);

  // initialise an instance of vpCameraParameters with the parameters from the tracker
  vpCameraParameters cam;
  tracker.getCameraParameters(cam);

  
  // Loop to position the cube
  if (opt_display && opt_click_allowed)
  {
    while(!vpDisplay::getClick(I,false)){
    vpDisplay::display(I);
    vpDisplay::displayCharString(I, 15, 10,
		 "click after positioning the object",
		 vpColor::red);
    vpDisplay::flush(I) ;
    }
  }

  // Load the 3D model (either a vrml file or a .cao file)
  try{
    tracker.loadModel(modelFile.c_str());
  }
  catch(...)
  {
    return 0;
  }
  // Initialise the tracker by clicking on the image
  // This function looks for 
  //   - a ./cube/cube.init file that defines the 3d coordinates (in meter, in the object basis) of the points used for the initialisation
  //   - a ./cube/cube.ppm file to display where the user have to click (optionnal, set by the third parameter)
  if (opt_display && opt_click_allowed)
  {
    tracker.initClick(I, initFile.c_str(), true);

    // display the 3D model at the given pose
    tracker.display(I,cMo, cam, vpColor::red);
  }
  else
  {
    vpHomogeneousMatrix cMoi(-0.002774173802,-0.001058705951,0.2028195729,2.06760528,0.8287820106,-0.3974327515);
    tracker.init(I,cMoi);
  }

    //track the model
  tracker.track(I);
  tracker.getPose(cMo);
  
  if (opt_display)
    vpDisplay::flush(I);

  int iter  = 0;
  while (iter < 200)
  {
    try
    {
      // acquire a new image 
      reader.acquire(I);
      // display the image
      if (opt_display)
        vpDisplay::display(I);
      // track the object
      tracker.track(I);
      tracker.getPose(cMo);
      // display the 3D model
      if (opt_display)
      {
        tracker.display(I, cMo, cam, vpColor::darkRed, 1);
        // display the frame
        vpDisplay::displayFrame (I, cMo, cam, 0.05, vpColor::blue);
      }
    }
    catch(...)
    {
      std::cout << "error caught" << std::endl;
      break;
    }
    vpDisplay::flush(I) ;
    iter++;
  }
  reader.close();
  return 0;
}

#else

int main()
{
  std::cout << "You should use OpenCV or a 1394 camera (can be set in the source file." << std::endl;
  return 0;
}

#endif
