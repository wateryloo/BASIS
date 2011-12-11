/**
 * @file  testdriver-itk4.hxx
 * @brief ITK-based implementation of test driver.
 *
 * This implementation of the test driver utilizes the ITK.
 *
 * This file is in parts a modified version of the itkTestDriverInclude.h
 * file which is part of the TestKernel module of the ITK 4 project and
 * in other parts contains code from the ImageCompareCommand.cxx file
 * which is part of the ITK 3.20 release.
 *
 * Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen<br />
 * Copyright (c) Insight Software Consortium.<br />
 * Copyright (c) 2011 University of Pennsylvania.
 *
 * Portions of this file are subject to the VTK Toolkit Version 3 copyright.
 *
 * For complete copyright, license and disclaimer of warranty information
 * please refer to the COPYRIGHT file.
 *
 * Contact: SBIA Group <sbia-software at uphs.upenn.edu>
 *
 * @ingroup CppUtilities
 */

 /*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
/*=========================================================================
 *
 *  Portions of this file are subject to the VTK Toolkit Version 3 copyright.
 *
 *  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 *
 *  For complete copyright, license and disclaimer of warranty information
 *  please refer to the NOTICE file at the top of the ITK source tree.
 *
 *=========================================================================*/

#pragma once
#ifndef _SBIA_BASIS_TESTDRIVER_ITK_HXX
#define _SBIA_BASIS_TESTDRIVER_ITK_HXX


// avoid dependency on all IO libraries which are commonly used
// has to be undefined before including the reader or writer
#ifdef ITK_IO_FACTORY_REGISTER_MANAGER
#  undef ITK_IO_FACTORY_REGISTER_MANAGER
#endif


#if ITK_VERSION_MAJOR >= 4
#  include <itkTestDriverInclude.h>
#else
#  include <itkImage.h>
#  include <itkImageFileReader.h>
#  include <itkImageFileWriter.h>
#  include <itkRescaleIntensityImageFilter.h>
#  include <itkExtractImageFilter.h>
#  include <itkDifferenceImageFilter.h>
#endif

#include <itkGDCMImageIOFactory.h>
#include <itkMetaImageIOFactory.h>
#include <itkJPEGImageIOFactory.h>
#include <itkPNGImageIOFactory.h>
#include <itkTIFFImageIOFactory.h>
#include <itkBMPImageIOFactory.h>
#include <itkVTKImageIOFactory.h>
#include <itkNrrdImageIOFactory.h>
#include <itkGiplImageIOFactory.h>
#include <itkNiftiImageIOFactory.h>
#include <itkObjectFactoryBase.h>


// maximum number of dimensions of test and baseline images
// Note: We cannot redefine the macro used by the ITK 4.
#if ITK_VERSION_MAJOR < 4
#  define ITK_TEST_DIMENSION_MAX BASIS_MAX_TEST_IMAGE_DIMENSION
#endif


// ---------------------------------------------------------------------------
void RegisterRequiredFactories()
{
    itk::ObjectFactoryBase::RegisterFactory(itk::MetaImageIOFactory::New());
    itk::ObjectFactoryBase::RegisterFactory(itk::GDCMImageIOFactory::New());
    itk::ObjectFactoryBase::RegisterFactory(itk::JPEGImageIOFactory::New());
    itk::ObjectFactoryBase::RegisterFactory(itk::VTKImageIOFactory::New());
    itk::ObjectFactoryBase::RegisterFactory(itk::PNGImageIOFactory::New());
    itk::ObjectFactoryBase::RegisterFactory(itk::TIFFImageIOFactory::New());
    itk::ObjectFactoryBase::RegisterFactory(itk::BMPImageIOFactory::New());
    itk::ObjectFactoryBase::RegisterFactory(itk::NrrdImageIOFactory::New());
    itk::ObjectFactoryBase::RegisterFactory(itk::GiplImageIOFactory::New());
    itk::ObjectFactoryBase::RegisterFactory(itk::NiftiImageIOFactory::New());
}


#if ITK_VERSION_MAJOR < 4


// ---------------------------------------------------------------------------
// This implementation of the image regression test was copied from the
// Testing/Code/IO/ImageCompareCommand.cxx file of the ITK 3.20 release.
// The function parameters have been changed such that the function prototype
// is identical to the ITK 4 version of this function above.
int RegressionTestImage (const char* testImageFilename,
                         const char* baselineImageFilename,
                         int reportErrors,
                         double intensityTolerance,
                         unsigned int radiusTolerance,
                         unsigned int numberOfPixelsTolerance)
{
  const bool createDifferenceImage = reportErrors > 0; // former function parameter

  // Use the factory mechanism to read the test and baseline files and convert them to double
  typedef itk::Image<double,ITK_TEST_DIMENSION_MAX>         ImageType;
  typedef itk::Image<unsigned char,ITK_TEST_DIMENSION_MAX>  OutputType;
  typedef itk::Image<unsigned char,2>                       DiffOutputType;
  typedef itk::ImageFileReader<ImageType>                   ReaderType;

  // Read the baseline file
  ReaderType::Pointer baselineReader = ReaderType::New();
    baselineReader->SetFileName(baselineImageFilename);
  try
    {
    baselineReader->UpdateLargestPossibleRegion();
    }
  catch (itk::ExceptionObject& e)
    {
    std::cerr << "Exception detected while reading " << baselineImageFilename << " : "  << e;
    return 1000;
    }

  // Read the file generated by the test
  ReaderType::Pointer testReader = ReaderType::New();
    testReader->SetFileName(testImageFilename);
  try
    {
    testReader->UpdateLargestPossibleRegion();
    }
  catch (itk::ExceptionObject& e)
    {
    std::cerr << "Exception detected while reading " << testImageFilename << " : "  << e << std::endl;
    return 1000;
    }

  // The sizes of the baseline and test image must match
  ImageType::SizeType baselineSize;
    baselineSize = baselineReader->GetOutput()->GetLargestPossibleRegion().GetSize();
  ImageType::SizeType testSize;
    testSize = testReader->GetOutput()->GetLargestPossibleRegion().GetSize();
  
  if (baselineSize != testSize)
    {
    std::cerr << "The size of the Baseline image and Test image do not match!" << std::endl;
    std::cerr << "Baseline image: " << baselineImageFilename
              << " has size " << baselineSize << std::endl;
    std::cerr << "Test image:     " << testImageFilename
              << " has size " << testSize << std::endl;
    return 1;
    }

  // Now compare the two images
  typedef itk::DifferenceImageFilter<ImageType,ImageType> DiffType;
  DiffType::Pointer diff = DiffType::New();
    diff->SetValidInput(baselineReader->GetOutput());
    diff->SetTestInput(testReader->GetOutput());
    
    diff->SetDifferenceThreshold( intensityTolerance );
    diff->SetToleranceRadius( radiusTolerance );

    diff->UpdateLargestPossibleRegion();

  bool differenceFailed = false;
  
  double averageIntensityDifference = diff->GetTotalDifference();

  unsigned long numberOfPixelsWithDifferences = 
                        diff->GetNumberOfPixelsWithDifferences();

  if( averageIntensityDifference > 0.0 )
    {
    if( numberOfPixelsWithDifferences > numberOfPixelsTolerance )
      {
      differenceFailed = true;
      }
    else
      {
      differenceFailed = false;
      }
    }
  else
    {
    differenceFailed = false; 
    }
  
  if (reportErrors)
    {
    typedef itk::RescaleIntensityImageFilter<ImageType,OutputType>    RescaleType;
    typedef itk::ExtractImageFilter<OutputType,DiffOutputType>        ExtractType;
    typedef itk::ImageFileWriter<DiffOutputType>                      WriterType;
    typedef itk::ImageRegion<ITK_TEST_DIMENSION_MAX>                  RegionType;

    OutputType::IndexType index; index.Fill(0);
    OutputType::SizeType size; size.Fill(0);

    RescaleType::Pointer rescale = RescaleType::New();

    rescale->SetOutputMinimum(itk::NumericTraits<unsigned char>::NonpositiveMin());
    rescale->SetOutputMaximum(itk::NumericTraits<unsigned char>::max());
    rescale->SetInput(diff->GetOutput());
    rescale->UpdateLargestPossibleRegion();

    RegionType region;
    region.SetIndex(index);
    
    size = rescale->GetOutput()->GetLargestPossibleRegion().GetSize();
    for (unsigned int i = 2; i < ITK_TEST_DIMENSION_MAX; i++)
      {
      size[i] = 0;
      }
    region.SetSize(size);

    ExtractType::Pointer extract = ExtractType::New();

    extract->SetInput(rescale->GetOutput());
    extract->SetExtractionRegion(region);

    WriterType::Pointer writer = WriterType::New();
      writer->SetInput(extract->GetOutput());
    if(createDifferenceImage)
      {
      // if there are discrepencies, create an diff image
      std::cout << "<DartMeasurement name=\"ImageError\" type=\"numeric/double\">";
      std::cout << averageIntensityDifference;
      std::cout <<  "</DartMeasurement>" << std::endl;

      std::cout << "<DartMeasurement name=\"NumberOfPixelsError\" type=\"numeric/int\">";
      std::cout << numberOfPixelsWithDifferences;
      std::cout <<  "</DartMeasurement>" << std::endl;

      itksys_ios::ostringstream diffName;
      diffName << testImageFilename << ".diff.png";
      try
        {
        rescale->SetInput(diff->GetOutput());
        rescale->Update();
        }
      catch(const std::exception& e)
        {
        std::cerr << "Error during rescale of " << diffName.str() << std::endl;
        std::cerr << e.what() << "\n";
        }
      catch (...)
        {
        std::cerr << "Error during rescale of " << diffName.str() << std::endl;
        }
      writer->SetFileName(diffName.str().c_str());
      try
        {
        writer->Update();
        }
      catch(const std::exception& e)
        {
        std::cerr << "Error during write of " << diffName.str() << std::endl;
        std::cerr << e.what() << "\n";
        }
      catch (...)
        {
        std::cerr << "Error during write of " << diffName.str() << std::endl;
        }

      std::cout << "<DartMeasurementFile name=\"DifferenceImage\" type=\"image/png\">";
      std::cout << diffName.str();
      std::cout << "</DartMeasurementFile>" << std::endl;
      }
    itksys_ios::ostringstream baseName;
    baseName << testImageFilename << ".base.png";
    try
      {
      rescale->SetInput(baselineReader->GetOutput());
      rescale->Update();
      }
    catch(const std::exception& e)
      {
      std::cerr << "Error during rescale of " << baseName.str() << std::endl;
      std::cerr << e.what() << "\n";
      }
    catch (...)
      {
      std::cerr << "Error during rescale of " << baseName.str() << std::endl;
      }
    try
      {
      writer->SetFileName(baseName.str().c_str());
      writer->Update();
      }
    catch(const std::exception& e)
      {
      std::cerr << "Error during write of " << baseName.str() << std::endl;
      std::cerr << e.what() << "\n";
      }
    catch (...)
      {
      std::cerr << "Error during write of " << baseName.str() << std::endl;
      }

    std::cout << "<DartMeasurementFile name=\"BaselineImage\" type=\"image/png\">";
    std::cout << baseName.str();
    std::cout << "</DartMeasurementFile>" << std::endl;

    itksys_ios::ostringstream testName;
    testName << testImageFilename << ".test.png";
    try
      {
      rescale->SetInput(testReader->GetOutput());
      rescale->Update();
      }
    catch(const std::exception& e)
      {
      std::cerr << "Error during rescale of " << testName.str() << std::endl;
      std::cerr << e.what() << "\n";
      }
    catch (...)
      {
      std::cerr << "Error during rescale of " << testName.str() << std::endl;
      }
    try
      {
      writer->SetFileName(testName.str().c_str());
      writer->Update();
      }
    catch(const std::exception& e)
      {
      std::cerr << "Error during write of " << testName.str() << std::endl;
      std::cerr << e.what() << "\n";
      }
    catch (...)
      {
      std::cerr << "Error during write of " << testName.str() << std::endl;
      }

    std::cout << "<DartMeasurementFile name=\"TestImage\" type=\"image/png\">";
    std::cout << testName.str();
    std::cout << "</DartMeasurementFile>" << std::endl;


    }
  return differenceFailed;
}


#endif // ITK_VERSION_MAJOR < 4


#endif // _SBIA_BASIS_TESTDRIVER_ITK_HXX