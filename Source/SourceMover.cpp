/*
  ==============================================================================

    SourceMover.cpp
    Created: 8 Aug 2014 1:04:53pm
    Author:  makira

  ==============================================================================
*/

#include "SourceMover.h"


SourceMover::SourceMover(ZirkOscjuceAudioProcessor *filter)
:
	mFilter(filter),
	mMover(kVacant)
{
    updateNumberOfSources();
}

void SourceMover::updateNumberOfSources(){
    mSourcesDownXY.clear();
    mSourcesDownRT.clear();
    mSourcesAngularOrder.clear();
    
    mSourcesDownXY.ensureStorageAllocated(mFilter->getNbrSources());
    mSourcesDownRT.ensureStorageAllocated(mFilter->getNbrSources());
    mSourcesAngularOrder.ensureStorageAllocated(mFilter->getNbrSources());
    
    for (int i = 0; i < mFilter->getNbrSources(); i++)
    {
        mSourcesDownXY.add(FPoint(0,0));
        mSourcesDownRT.add(FPoint(0,0));
        mSourcesAngularOrder.add(0);
    }
}

/** Begin is use to make sure the mouvements done by a source are complete and well ended by the end method. To do so it prepares everything that could be modified. */

void SourceMover::begin(int s, MoverType mt)
{
	if (mMover != kVacant) return;
	mMover = mt;
	mSelectedItem = s;
	
	mFilter->beginParameterChangeGesture(mFilter->getParamForSourceX(mSelectedItem));
	mFilter->beginParameterChangeGesture(mFilter->getParamForSourceY(mSelectedItem));

	if (mFilter->getMovementMode() != 0 && mFilter->getNbrSources() > 1 && mFilter->getLinkMovement())
	{
		for (int j = 0; j < mFilter->getNbrSources(); j++)
		{
			mSourcesDownRT.setUnchecked(j, mFilter->getSourceRT(j));
			mSourcesDownXY.setUnchecked(j, mFilter->getSourceXY(j));
			if (j == mSelectedItem) continue;
			mFilter->beginParameterChangeGesture(mFilter->getParamForSourceX(j));
			mFilter->beginParameterChangeGesture(mFilter->getParamForSourceY(j));
		}
		
		if	(	(mFilter->getNbrSources() == 2 && (mFilter->getMovementMode() == 6 || mFilter->getMovementMode() == 7))
			 ||	(mFilter->getNbrSources() >  2 && (mFilter->getMovementMode() == 3 || mFilter->getMovementMode() == 4))
			 )
		{
			// need to calculate angular order
			
			//IndexedAngle ia[mFilter->getNumberOfSources()];
			IndexedAngle * ia = new IndexedAngle[mFilter->getNbrSources()];
			
			for (int j = 0; j < mFilter->getNbrSources(); j++)
			{
				ia[j].i = j;
				ia[j].a = mFilter->getSourceRT(j).y;
			}
			
			//printf("==============\nbefore sort:\n");
			//for (int j = 0; j < mNumberOfSources; j++) printf("ia[%i] = { %i, %.3f }\n", j+1, ia[j].i+1, ia[j].a);
			
			qsort(ia, mFilter->getNbrSources(), sizeof(IndexedAngle), IndexedAngleCompare);
			
			//printf("after sort:\n");
			//for (int j = 0; j < mNumberOfSources; j++) printf("ia[%i] = { %i, %.3f }\n", j+1, ia[j].i+1, ia[j].a);
			
			int b;
			for (b = 0; b < mFilter->getNbrSources() && ia[b].i != mSelectedItem; b++) ;
			
			if (b == mFilter->getNbrSources())
			{
				printf("error!\n");
				b = 0;
			}
			
			//printf("mSelectedItem: %i base: %i step: %.3f\n", mSelectedItem+1, b+1, (M_PI * 2.) / mNumberOfSources);
			
			for (int j = 1; j < mFilter->getNbrSources(); j++)
			{
				int o = (b + j) % mFilter->getNbrSources();
				o = ia[o].i;
				mSourcesAngularOrder.set(o, (M_PI * 2. * j) / mFilter->getNbrSources());
			}
			
			//for (int j = 0; j < mNumberOfSources; j++)  printf("mSourceAngularOrder[%i] = %.3f\n", j+1, mSourceAngularOrder[j]);

			delete[] ia;
		}
	}
	
}

/** Move is the actual method that will change the data in order to create the move, it's also handling all the modification implied by the selected movement mode. */

void SourceMover::move(FPoint p, MoverType mt)
{
	if (mMover != mt) return;
	
	float vx = p.x, vy = p.y;
	mFilter->setSourceXY01(mSelectedItem, FPoint(vx, vy));
	
	if (mFilter->getMovementMode() != 0 && mFilter->getNbrSources() == 2 && mFilter->getLinkMovement())
	{
		int otherItem = 1 - mSelectedItem;
		float vxo = vx, vyo = vy;
		switch(mFilter->getMovementMode())
		{
			case 1: // sym x
				vyo = 1 - vyo;
				mFilter->setSourceXY01(otherItem, FPoint(vxo, vyo));
				break;
				
			case 2: // sym y
				vxo = 1 - vxo;
				mFilter->setSourceXY01(otherItem, FPoint(vxo, vyo));
				break;
				
			case 3: // sym x/y
				vxo = 1 - vxo;
				vyo = 1 - vyo;
				mFilter->setSourceXY01(otherItem, FPoint(vxo, vyo));
				break;
				
			case 4: // circular
				{
					FPoint s = mSourcesDownRT[mSelectedItem];
					FPoint o = mSourcesDownRT[otherItem];
					FPoint n = o + mFilter->getSourceRT(mSelectedItem) - s;
					if (n.x < 0) n.x = 0;
					if (n.x > kRadiusMax) n.x = kRadiusMax;
					if (n.y < 0) n.y += kThetaMax;
					if (n.y > kThetaMax) n.y -= kThetaMax;
					mFilter->setSourceRT(otherItem, n);
				}
				break;
				
			case 5: // circular, fixed radius
				{
					FPoint s = mSourcesDownRT[mSelectedItem];
					FPoint o = mSourcesDownRT[otherItem];
					FPoint sn = mFilter->getSourceRT(mSelectedItem);
					FPoint n = o + sn - s;
					n.x = sn.x;
					if (n.y < 0) n.y += kThetaMax;
					if (n.y > kThetaMax) n.y -= kThetaMax;
					mFilter->setSourceRT(otherItem, n);
				}
				break;
				
			case 6: // circular, fixed angle
				{
					FPoint s = mSourcesDownRT[mSelectedItem];
					FPoint o = mSourcesDownRT[otherItem];
					FPoint sn = mFilter->getSourceRT(mSelectedItem);
					FPoint n = o + sn - s;
					n.y = sn.y + mSourcesAngularOrder[otherItem];
					if (n.x < 0) n.x = 0;
					if (n.x > kRadiusMax) n.x = kRadiusMax;
					if (n.y < 0) n.y += kThetaMax;
					if (n.y > kThetaMax) n.y -= kThetaMax;
					mFilter->setSourceRT(otherItem, n);
				}
				break;
				
			case 7: // circular, fully fixed
				{
					FPoint s = mSourcesDownRT[mSelectedItem];
					FPoint o = mSourcesDownRT[otherItem];
					FPoint sn = mFilter->getSourceRT(mSelectedItem);
					FPoint n = o + sn - s;
					n.x = sn.x;
					n.y = sn.y + mSourcesAngularOrder[otherItem];
					if (n.y < 0) n.y += kThetaMax;
					if (n.y > kThetaMax) n.y -= kThetaMax;
					mFilter->setSourceRT(otherItem, n);
				}
				break;
				
			case 8: // delta lock
				{
					FPoint d = mFilter->getSourceXY(mSelectedItem) - mSourcesDownXY[mSelectedItem];
					mFilter->setSourceXY(otherItem, mSourcesDownXY[otherItem] + d);
				}
				break;
		}
		
	}
	else if (mFilter->getMovementMode() != 0 && mFilter->getNbrSources() > 2 && mFilter->getLinkMovement())
	{
		for (int otherItem = 0; otherItem < mFilter->getNbrSources(); otherItem++)
		{
			if (otherItem == mSelectedItem) continue;
			
			switch(mFilter->getMovementMode())
			{
				case 1: // circular
					{
						FPoint s = mSourcesDownRT[mSelectedItem];
						FPoint o = mSourcesDownRT[otherItem];
						FPoint n = o + mFilter->getSourceRT(mSelectedItem) - s;
						if (n.x < 0) n.x = 0;
						if (n.x > kRadiusMax) n.x = kRadiusMax;
						if (n.y < 0) n.y += kThetaMax;
						if (n.y > kThetaMax) n.y -= kThetaMax;
						mFilter->setSourceRT(otherItem, n);
					}
					break;
					
				case 2: // circular, fixed radius
					{
						FPoint s = mSourcesDownRT[mSelectedItem];
						FPoint o = mSourcesDownRT[otherItem];
						FPoint sn = mFilter->getSourceRT(mSelectedItem);
						FPoint n = o + sn - s;
						n.x = sn.x;
						if (n.y < 0) n.y += kThetaMax;
						if (n.y > kThetaMax) n.y -= kThetaMax;
						mFilter->setSourceRT(otherItem, n);
					}
					break;
					
				case 3: // circular, fixed angle
					{
						FPoint s = mSourcesDownRT[mSelectedItem];
						FPoint o = mSourcesDownRT[otherItem];
						FPoint sn = mFilter->getSourceRT(mSelectedItem);
						FPoint n = o + sn - s;
						n.y = sn.y + mSourcesAngularOrder[otherItem];
						if (n.x < 0) n.x = 0;
						if (n.x > kRadiusMax) n.x = kRadiusMax;
						if (n.y < 0) n.y += kThetaMax;
						if (n.y > kThetaMax) n.y -= kThetaMax;
						mFilter->setSourceRT(otherItem, n);
					}
					break;
					
				case 4: // circular, fully fixed
					{
						FPoint s = mSourcesDownRT[mSelectedItem];
						FPoint o = mSourcesDownRT[otherItem];
						FPoint sn = mFilter->getSourceRT(mSelectedItem);
						FPoint n = o + sn - s;
						n.x = sn.x;
						n.y = sn.y + mSourcesAngularOrder[otherItem];
						if (n.y < 0) n.y += kThetaMax;
						if (n.y > kThetaMax) n.y -= kThetaMax;
						mFilter->setSourceRT(otherItem, n);
					}
					break;
					
				case 5: // delta lock
					{
						FPoint d = mFilter->getSourceXY(mSelectedItem) - mSourcesDownXY[mSelectedItem];
						mFilter->setSourceXY(otherItem, mSourcesDownXY[otherItem] + d);
					}
					break;
			}
		}
	}
}

/** End is the method that makes sure that everything is ok the way it has been modified. */
void SourceMover::end(MoverType mt)
{
	if (mMover != mt) return;
	
	mFilter->endParameterChangeGesture(mFilter->getParamForSourceX(mSelectedItem));
	mFilter->endParameterChangeGesture(mFilter->getParamForSourceY(mSelectedItem));
	if (mFilter->getMovementMode() != 0 && mFilter->getNbrSources() > 1 && mFilter->getLinkMovement())
	{
		for (int i = 0; i < mFilter->getNbrSources(); i++)
		{
			if (i == mSelectedItem) continue;
			mFilter->endParameterChangeGesture(mFilter->getParamForSourceX(i));
			mFilter->endParameterChangeGesture(mFilter->getParamForSourceY(i));
		}
	}
	
	mMover = kVacant;
}
