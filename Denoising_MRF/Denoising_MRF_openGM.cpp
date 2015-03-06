/***********************************************************************
*
*
 ************************************************************************/

#include <iostream>
#include "/home/lex/opencv-3.0.0-beta/include/opencv2/opencv.hpp"
#include <opengm/opengm.hxx>
#include <opengm/graphicalmodel/graphicalmodel.hxx>
#include <opengm/operations/adder.hxx>
#include <opengm/functions/potts.hxx>
#include <opengm/functions/absolute_difference.hxx>
#include <opengm/inference/messagepassing/messagepassing.hxx>
#include <opengm/inference/gibbs.hxx>
//#include <opengm/inference/graphcut.hxx>
//#include <opengm/inference/auxiliary/minstcutkolmogorov.hxx>
//#include <opengm/operations/minimizer.hxx>

#include <stdio.h>



using namespace cv;
using namespace std;
using namespace opengm;

int main() {

   
    //*******************
    //**Images
    //*******************	
    cv::Mat img_src;//source image
    cv::Mat img_out;//output image
    cv::Mat img_srcGray;//source image gray scaled
    cv::Mat img_srcBinary;//source image binary	


   //*******************
   //** Typedefs
   //*******************
   typedef double                                                                 ValueType;          // type used for values
   typedef size_t                                                                 IndexType;          // type used for indexing nodes and factors (default : size_t)
   typedef size_t                                                                 LabelType;          // type used for labels (default : size_t)
   typedef opengm::Adder                                                          OpType;             // operation used to combine terms
   typedef opengm::ExplicitFunction<ValueType,IndexType,LabelType>                ExplicitFunction;   // shortcut for explicite function 
   typedef opengm::PottsFunction<ValueType,IndexType,LabelType>                   PottsFunction;      // shortcut for Potts function
  
   typedef opengm::meta::TypeListGenerator<ExplicitFunction,PottsFunction>::type  FunctionTypeList;   // list of all function the model cal use (this trick avoids virtual methods) - here only one
   typedef opengm::DiscreteSpace<IndexType, LabelType>                            SpaceType;          // type used to define the feasible statespace
   typedef opengm::GraphicalModel<ValueType,OpType,FunctionTypeList,SpaceType>    Model;              // type of the model
   typedef Model::FunctionIdentifier                                              FunctionIdentifier; // type of the function identifier

    //**********************
    //**image preprocessing
    //**********************

    img_src=cv::imread("/home/lex/2_Sistemic/3_Markov_random_fields/3_images/noisy.png");
    img_srcGray=cv::imread("/home/lex/2_Sistemic/3_Markov_random_fields/3_images/noisy.png",0);

    //cv::cvtColor(img_src,img_srcGray,cv::COLOR_RGB2GRAY);//chenge to gray scale
    cv::threshold(img_srcGray,img_srcBinary,0,255,cv::THRESH_OTSU);//binarize  image
    cv::namedWindow("In",0);	 
    cv::imshow("In",img_srcBinary);//show image
    cv::waitKey(0);


   //******************
   //** DATA
   //******************
   IndexType N =img_src.cols;
   IndexType M = img_src.rows;  
   int data[N*M];	
   int countData=0;	

	for(int i=0;i<N;i++)
	{
		for(int j=0;j<M;j++)
		{
		
		if(img_srcBinary.at<uchar>(i,j)>0)data[countData]=0;
		if(img_srcBinary.at<uchar>(i,j)<1)data[countData]=1;


		//data[countData]=img_srcBinary.at<uchar>(i,j);
 		//std::cout <<data[countData];
		countData++;	
		}	

	}




   //*******************
   //** Code
   //*******************

   std::cout << "Start building the model ... "<<std::endl;
   // Build empty Model
   LabelType numLabel = 2;
   std::vector<LabelType> numbersOfLabels(N*M,numLabel);
   Model gm(SpaceType(numbersOfLabels.begin(), numbersOfLabels.end()));

   // Add 1st order functions and factors to the model
   for(IndexType variable = 0; variable < gm.numberOfVariables(); ++variable) {
      // construct 1st order function
      const LabelType shape[] = {gm.numberOfLabels(variable)};
      ExplicitFunction f(shape, shape + 1);
      f(0) =  std::fabs(1 - data[variable]);
      f(1) = std::fabs(data[variable]);
      // add function
      FunctionIdentifier id = gm.addFunction(f);
      // add factor
      IndexType variableIndex[] = {variable};
      gm.addFactor(id, variableIndex, variableIndex + 1);
   }
   // add 2nd order functions for all variables neighbored on the grid
	//las funciones de segundo orden son las funciones de enrgia entre pixeles
	
   {





        opengm::AbsoluteDifferenceFunction<float> Absolute(numLabel, numLabel,0.5);
	//AbsoluteDifferenceFunction Absolute(numLabel, numLabel,0.5);
      // add a potts function to the model
      PottsFunction potts(numLabel, numLabel, 0.0, 2.0);
      FunctionIdentifier pottsid = gm.addFunction(potts);

      IndexType vars[]  = {0,1}; 
      for(IndexType n=0; n<N;++n){
         for(IndexType m=0; m<M;++m){
            vars[0] = n + m*N;
            if(n+1<N){ //check for right neighbor
               vars[1] =  (n+1) + (m  )*N;
               OPENGM_ASSERT(vars[0] < vars[1]); // variables need to be ordered!
               gm.addFactor(pottsid, vars, vars + 2);
            } 
            if(m+1<M){ //check for lower neighbor
               vars[1] =  (n  ) + (m+1)*N; 
               OPENGM_ASSERT(vars[0] < vars[1]); // variables need to be ordered!
               gm.addFactor(pottsid, vars, vars + 2);
            }
         }
      }
   }

	/*typedef opengm::external::MinSTCutKolmogorov<size_t, double> MinCut;
	typedef opengm::GraphCut<GraphicalModelType, opengm::Minimizer, MinStCutType> MinCut;

	MinCut mimcut(gm);
	mincut.infer();
	std::cout<<"value"<<mincut.value()<<std::endl;*/

    // set up the optimizer (loopy belief propagation)
   typedef BeliefPropagationUpdateRules<Model, opengm::Minimizer> UpdateRules;
   typedef MessagePassing<Model, opengm::Minimizer, UpdateRules, opengm::MaxDistance> BeliefPropagation;
   const size_t maxNumberOfIterations = 30;
   const double convergenceBound = 1e-7;
   const double damping = 0.0;
   BeliefPropagation::Parameter parameter(maxNumberOfIterations, convergenceBound, damping);
   BeliefPropagation bp(gm, parameter);
   
   // optimize (approximately)
   BeliefPropagation::VerboseVisitorType visitor;
   bp.infer(visitor);

   // obtain the (approximate) argmin
   vector<size_t> labeling(N * M);
   bp.arg(labeling);


   cv::Mat Resultado;
img_srcBinary.copyTo(Resultado);
	
   
   // output the (approximate) argmin
   size_t variableIndex = 0;
int i=0;int j=0;
   for(size_t y = 0; y < M; ++y) {

      for(size_t x = 0; x < N; ++x) {


	cout<<labeling[variableIndex]<<" ";


	if(labeling[variableIndex]>0)Resultado.at<uchar>(i,j)=255;
	if((int)(labeling[variableIndex])<1)Resultado.at<uchar>(y,x)=0;

        // cout << labeling[variableIndex] << ' ';
	/*if((int)(labeling[variableIndex])==1 && i<img_srcBinary.rows && j<img_srcBinary.cols )
	{
	Resultado.at<uchar>(i,j)=255;
	//cout<<labeling[variableIndex]<<" ";
	
	
	}*/

/*	if(labeling[variableIndex]> 1421211){
	Resultado.at<uchar>(i,j)=0;
	//cout<<labeling[variableIndex]<<" ";
	}*/


	//cout<<"["<<i<<" "<<j<<"]"<<" ";
	//cout<<img_srcBinary.rows<<img_srcBinary.cols;
         ++variableIndex;
	
	j++;
      }
	j=0;
	i++;   
      
   }

	cv::namedWindow("out",0);

	cv::Mat resta;
  	cv::subtract(img_srcBinary,Resultado,resta);	 
    cv::imshow("out",Resultado);//show image
    cv::waitKey(0);

  /* // View some model information
   std::cout << "The model has " << gm.numberOfVariables() << " variables."<<std::endl;
   for(size_t i=0; i<gm.numberOfVariables(); ++i){
      std::cout << " * Variable " << i << " has "<< gm.numberOfLabels(i) << " labels."<<std::endl; 
   } 
   std::cout << "The model has " << gm.numberOfFactors() << " factors."<<std::endl;
   /*for(size_t f=0; f<gm.numberOfFactors(); ++f){
      std::cout << " * Factor " << f << " has order "<< gm[f].numberOfVariables() << "."<<std::endl; 
   }

   LabelType labelA[] = {0,0,0,0,0,0, 0,0,0,0,0,0, 1,1,1,1,1,1, 1,1,1,1,1,1};
   LabelType labelB[] = {0,0,0,0,0,0, 0,0,0,0,0,0, 0,0,0,0,0,0, 0,0,0,0,0,0}; 
   LabelType labelC[] = {1,1,1,1,1,1, 1,1,1,1,1,1, 1,1,1,1,1,1, 1,1,1,1,1,1};
   std::cout << "The Labeling x_A  has the energy "<<gm.evaluate(labelA)<<"."<<std::endl;
   std::cout << "The Labeling x_B  has the energy "<<gm.evaluate(labelB)<<"."<<std::endl;
   std::cout << "The Labeling x_C  has the energy "<<gm.evaluate(labelC)<<"."<<std::endl;*/
	
   std::cout <<"Acabe";

	return 0;

}
