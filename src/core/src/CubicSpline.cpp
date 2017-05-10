/*!
 * @file CubicSpline.cpp
 * @author Stefano Dafarra
 * @copyright 2016 iCub Facility - Istituto Italiano di Tecnologia
 *            Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 * @date 2017
 *
 */

#include <iDynTree/Core/CubicSpline.h>
#include <iDynTree/Core/EigenHelpers.h>
#include <Eigen/Dense>
#include <iostream>
#include <cmath>


iDynTree::CubicSpline::CubicSpline()
:m_v0(0)
,m_vf(0)
,m_a0(0)
,m_af(0)
{
    m_coefficients.clear();
    m_velocities.resize(0);
    m_time.resize(0);
    m_y.resize(0);
    m_T.resize(0);;
}

iDynTree::CubicSpline::~CubicSpline()
{}



bool iDynTree::CubicSpline::setData(const iDynTree::VectorDynSize& time, const iDynTree::VectorDynSize& yData)
{
    if((time.size() == 0) && (yData.size() == 0)){
        std::cerr << "[CUBICSPLINE] The input data are empty!" << std::endl;
        return false;
    }
    
    if(time.size() != yData.size()){
        std::cerr << "[CUBICSPLINE] The input data are expected to have the same dimension: xData = " << time.size() << ", yData = " << yData.size() << "." <<std::endl;
        return false;
    }
    
    if(time.size() < 2){
        std::cerr << "[CUBICSPLINE] At least two data points are needed to compute the spline." << std::endl;
        return false;
    }

    m_time.resize(time.size());
    m_y.resize(yData.size());
    
    m_coefficients.resize(time.size()-1);
    m_velocities.resize(time.size());
    
    m_T.resize(time.size()-1);
    
    m_time = time;

    m_y = yData;
    
    return this->computeCoefficients();
}

bool iDynTree::CubicSpline::computeCoefficients()
{
    m_velocities(0) = m_v0;
    m_velocities(m_velocities.size() - 1) = m_vf;
    
    if(!this->computePhasesDuration())
        return false;
    
    if(m_velocities.size() > 2){
        if(!this->computeIntermediateVelocities())
            return false;
    }
    
    for(size_t i = 0; i < m_coefficients.size(); ++i){
        m_coefficients[i](0) = m_y(i);
        m_coefficients[i](1) = m_velocities(i);
        m_coefficients[i](2) = ( 3*(m_y(i+1) - m_y(i))/m_T(i) - 2*m_velocities(i) - m_velocities(i+1) )/m_T(i);
        m_coefficients[i](3) = ( 2*(m_y(i) - m_y(i+1))/m_T(i) + m_velocities(i) + m_velocities(i+1) )/std::pow(m_T(i), 2);
    }
    
    return true;
}

bool iDynTree::CubicSpline::computeIntermediateVelocities()
{
    Eigen::MatrixXd A(m_velocities.size()-2, m_velocities.size()-2);
    Eigen::VectorXd b(m_velocities.size()-2);
    
    A.setZero();
    
    A(0,0) = 2*(m_T(0) + m_T(1));
    b(0) = ( std::pow(m_T(0), 2)*(m_y(2) - m_y(1)) + std::pow(m_T(1), 2)*(m_y(1) - m_y(0)) )*3/(m_T(0)*m_T(1)) - m_T(1)*m_velocities(0);
    
    if(m_velocities.size() > 3){
        A(0,1) = m_T(0);
        for(int i = 1; i < A.rows()-1; ++i){
            A(i,i-1) = m_T(i+1);
            A(i,i) = 2*(m_T(i) + m_T(i+1));
            A(i,i+1) = m_T(i);
            
            b(i) = ( std::pow(m_T(i), 2)*(m_y(i+2) - m_y(i+1)) + std::pow(m_T(i+1), 2)*(m_y(i+1) - m_y(i)) )*3/(m_T(i)*m_T(i+1));
        }
        size_t T = m_T.size() - 1; 
        A.bottomRightCorner<1,2>() << m_T(T), 2*(m_T(T) + m_T(T-1));
        b.tail<1>() << ( std::pow(m_T(T-1), 2)*(m_y(T+1) - m_y(T)) + std::pow(m_T(T), 2)*(m_y(T) - m_y(T-1)) )*3/(m_T(T-1)*m_T(T)) - m_T(T-1)*m_velocities(T+1);
    }
    
   iDynTree::toEigen(m_velocities).segment(1, m_velocities.size()-2) = A.colPivHouseholderQr().solve(b);
    
    return true;
}

bool iDynTree::CubicSpline::computePhasesDuration()
{
    for (size_t i = 0; i < m_time.size() - 1; ++i){
        
        m_T(i) = m_time(i+1) - m_time(i);
        
        if(m_T(i) == 0){
            std::cerr << "[CUBICSPLINE] Two consecutive points have the same time coordinate." << std::endl; //For stability purposes, the matrix below may not be invertible
            return false;
        }
        
        if(m_T(i) < 0){
            std::cerr << "[CUBICSPLINE] The input points are expected to be consecutive, strictly increasing in the time variable." << std::endl; //For stability purposes
            return false;
        }
        
    }
    return true;
}

void iDynTree::CubicSpline::setInitialConditions(double initialVelocity, double initialAcceleration)
{
    m_v0 = initialVelocity;
    m_a0 = initialAcceleration;
}

void iDynTree::CubicSpline::setFinalConditions(double finalVelocity, double finalAcceleration)
{
    m_vf = finalVelocity;
    m_af = finalAcceleration;
}

double iDynTree::CubicSpline::evaluatePoint(double t)
{
    if(m_time.size() == 0){
        std::cerr << "[CUBICSPLINE] First you have to load data! The returned data should not be considered." << std::endl; 
        return 1e19;
    }
    
    if( t < m_time(0) )
        return m_y(0);
    
    if( t >= m_time(m_time.size()-1))
        return m_y(m_y.size()-1);
    
    size_t coeffIndex = 0;
    while( (coeffIndex < m_time.size()) && (t >= m_time(coeffIndex)) ){
        coeffIndex++;
    }
    coeffIndex--; //Actually we are interested in the last index for which t >= m_time(coeffIndex) holds
    
    iDynTree::Vector4 coeff = m_coefficients[coeffIndex];
    double dt = t - m_time(coeffIndex);
    
    return coeff(0) + coeff(1)*(dt) + coeff(2)*std::pow(dt,2) + coeff(3)*std::pow(dt,3);
}
