#include"CCity.h"
#include<GL/gl.h>
#include<iostream>
using namespace std;
#include <glm/gtc/type_ptr.hpp>


CStreet::CStreet(glm::vec3 Start,glm::vec3 End,float Width){
	this->Start=Start;
	this->End=End;
	this->Width=Width;
}

void CStreet::Split(CStreet**A,CStreet**B,float t){
	glm::vec3 Midpoint=glm::mix(this->Start,this->End,t);
	*A=new CStreet(this->Start,Midpoint,this->Width);
	*B=new CStreet(Midpoint,this->End,this->Width);
}

float CStreet::Length(){
	glm::length(this->End-this->Start);
}

glm::vec3 CStreet::GetPos(float t){
	return (1-t)*this->Start+(t)*this->End;
}


int CStreetNet::IsStreetCollision(CStreet*A,CStreet*M,float*t){
	/*
	X=A+t(B-A)
	X=M+s(N-M)
	X=A+tC
	X=M+sO
	A+tC=M+sO
	tC-sO+A-M=0
	tC-sO+H=0
	tC+sP+H=0
	C=B-A
	P=M-N
	H=A-M
	tCx+sPx+Hx=0
	tCz+sPz+Hz=0
	t=-(HzPx-HxPz)/(CzPx-CxPz)
	s=-(CzHx-CxHz)/(CzPx-CxPz)
	t=-R/S
	s=-X/S
	*/
	glm::vec3 C=A->End-A->Start;
	glm::vec3 P=M->Start-M->End;
	glm::vec3 H=A->Start-M->Start;
	float R=H[2]*P[0]-H[0]*P[2];
	float X=C[2]*H[0]-C[0]*H[2];
	float S=C[2]*P[0]-C[0]*P[2];
	t[0]=-R/S;
	t[1]=-X/S;
	return t[0]>=0&&t[0]<=1&&t[1]>=0&&t[1]<=1;
}

CStreetNet::CStreetNet(SStreetNetTemplate T){
	gen_Seed(T.Seed);
	CStreet*ActStreet;
	for(unsigned s=0;s<T.NumStreet;++s){
		glm::vec3 Start,End;
		if(s>0){
			unsigned Base;
			do{
				Base=gen_Uniform(0,1)*s;
			}while(this->Streets[Base]->Length()<T.MinSplitLen);
			float t=gen_Uniform(.2,.8);
			Start=this->Streets[Base]->GetPos(t);// starting point of the new street
			do{
				End=glm::vec3(gen_Uniform(0,1),0,gen_Uniform(0,1));
			}while(glm::length(End-Start)<T.MinLen||
					1-glm::abs(glm::dot(this->Streets[Base]->End-this->Streets[Base]->Start,End-Start)/
					this->Streets[Base]->Length()/glm::length(End-Start))<T.MinAngle);

			ActStreet=new CStreet(Start,End,T.StreetWidth.Value());

			float tmin[2]={1,1};//Where was the collision
			float smin=1;
			for(unsigned i=0;i<this->Streets.size();++i){//cycle for collisition
				if(i==Base)continue;
				if(this->IsStreetCollision(this->Streets[i],ActStreet,tmin)){
					smin=glm::min(smin,tmin[1]);
				}
			}
			ActStreet->End-=(1-smin)*(ActStreet->End-ActStreet->Start);
			this->Streets.push_back(ActStreet);
			CStreet*A,*B;
			this->Streets[Base]->Split(&A,&B,t);
			delete this->Streets[Base];
			this->Streets[Base]=A;
			this->Streets.push_back(B);
		}else{
			do{
				Start=glm::vec3(gen_Uniform(0,1),0,gen_Uniform(0,1));
				End=glm::vec3(gen_Uniform(0,1),0,gen_Uniform(0,1));
			}while(glm::length(End-Start)<T.MinLen);
			ActStreet=new CStreet(Start,End,T.StreetWidth.Value());
			this->Streets.push_back(ActStreet);
		}
	}
}

CStreetNet::~CStreetNet(){
	for(unsigned i=0;i<this->Streets.size();++i)
		delete this->Streets[i];
	this->Streets.clear();
}

void CStreetNet::Draw(){
	glColor3f(1,0,0);
	for(unsigned i=0;i<this->Streets.size();++i){
		//glLineWidth(this->Streets[0]->Width);
		glBegin(GL_LINES);
	 		glVertex3f(this->Streets[i]->Start[0],this->Streets[i]->Start[1],this->Streets[i]->Start[2]);
			glVertex3f(this->Streets[i]->End[0],this->Streets[i]->End[1],this->Streets[i]->End[2]);
		glEnd();
	}
	for(unsigned i=0;i<this->Streets.size();++i){
		glColor3f(1,1,1);
		glPointSize(2);
		glBegin(GL_POINTS);
	 		glVertex3f(this->Streets[i]->Start[0],this->Streets[i]->Start[1],this->Streets[i]->Start[2]);
			glVertex3f(this->Streets[i]->End[0],this->Streets[i]->End[1],this->Streets[i]->End[2]);
		glEnd();
	}

}

CBuilding::CBuilding(glm::vec3 Start,glm::vec3 Size,glm::vec3 Xvec){
	this->Start=Start;
	this->Size[0]=Xvec*Size[0];
	this->Size[1]=glm::vec3(0,1,0)*Size[1];
	this->Size[2]=glm::vec3(-Xvec[2],0,Xvec[0])*Size[2];
}

void CBuilding::Draw(){
	glColor3f(1,1,0);
	glm::vec3 Corner[8];
	for(unsigned i=0;i<8;++i){
		Corner[i]=this->Start;
		for(unsigned b=0;b<3;++b)
			Corner[i]+=(this->Size[b]*((float)((i>>b)&1)));
	}
	glBegin(GL_LINE_LOOP);
	 	glVertex3fv(glm::value_ptr(Corner[0]));
		glVertex3fv(glm::value_ptr(Corner[1]));
		glVertex3fv(glm::value_ptr(Corner[5]));
		glVertex3fv(glm::value_ptr(Corner[4]));
	glEnd();

}

CCity::CCity(CStreetNet*Net,SBuildingTemplate T){
	vector<CBuilding*>Carry;
	for(unsigned s=0;s<Net->Streets.size();++s){//cycle over streets
		float t1=0;
		float t2=0;
		glm::vec3 X=glm::normalize(Net->Streets[s]->End-Net->Streets[s]->Start);
		glm::vec3 Z=glm::vec3(-X[2],0,X[0]);
		do{
			glm::vec3 Size1=glm::vec3(T.XSize.Value(),T.YSize.Value(),T.ZSize.Value());
			glm::vec3 Start1=Net->Streets[s]->Start+t1*X+Z*Net->Streets[s]->Width;
			Carry.push_back(new CBuilding(Start1,Size1,X));
			t1+=Size1[0]+T.XSize.Value()/2;
		}while(t1<Net->Streets[s]->Length());
		do{
			glm::vec3 Size2=glm::vec3(T.XSize.Value(),T.YSize.Value(),T.ZSize.Value());
			glm::vec3 Start2=Net->Streets[s]->Start+t2*X-Z*(Net->Streets[s]->Width+Size2[2]);
			Carry.push_back(new CBuilding(Start2,Size2,X));
			t2+=Size2[0]+T.XSize.Value()/2;
		}while(t2<Net->Streets[s]->Length());
	}
	for(unsigned i=0;i<Carry.size();++i){
		int Collision=0;
		for(unsigned j=0;j<this->Buildings.size();++j)
			Collision|=this->Buildings[j]->IsCollision(Carry[i]);
		for(unsigned s=0;s<Net->Streets.size();++s)
			Collision|=Carry[i]->IsCollision(Net->Streets[s]);
		if(!Collision)
			this->Buildings.push_back(Carry[i]);
		else
			delete Carry[i];
	}
	Carry.clear();
	this->Net=Net;
}

int CBuilding::IsCollision(CBuilding*B){
	glm::vec3 ThisCenter=(this->Size[0]+this->Size[1]+this->Size[2]);
	ThisCenter*=0.5;
	ThisCenter+=this->Start;
	glm::vec3 BCenter=(B->Size[0]+B->Size[1]+B->Size[2]);
	BCenter*=0.5;
	BCenter+=B->Start;
	float r1=glm::length((this->Size[0]+this->Size[1]+this->Size[2]))/2;
	float r2=glm::length((B->Size[0]+B->Size[1]+B->Size[2]))/2;
	return glm::length(ThisCenter-BCenter)<=r1+r2;
}

int CBuilding::IsCollision(CStreet*S){
	glm::vec3 P=(this->Size[0]+this->Size[1]+this->Size[2]);
	float r=glm::length(P)/2;
	P*=0.5;
	P+=this->Start;
	glm::vec3 C=S->End-S->Start;
	glm::vec3 V=P-S->Start;
	float a=glm::dot(C,C);
	float b=2*glm::dot(V,C);
	float c=glm::dot(V,V)-r*r;
	float d=b*b-4*a*c;
	if(d<0)return 0;
	float t1=(-b-sqrt(d))/2/a;
	float t2=(-b+sqrt(d))/2/a;
	if(t1>1||t2<0)return 0;
	if(t1<0&&t2>=0)return 1;
	if(t1<0&&t2<0)return 0;
	if(t1>=0&&t1<=1)return 1;
	if(t2>=0&&t2<=1)return 1;
	if(t1>1)return 0;
}

CCity::~CCity(){
	this->Net->~CStreetNet();
	for(unsigned i=0;i<this->Buildings.size();++i)
		delete this->Buildings[i];
	this->Buildings.clear();
}

void CCity::Draw(){
	this->Net->Draw();
	for(unsigned i=0;i<this->Buildings.size();++i)
		this->Buildings[i]->Draw();
}
