struct VSOut
{
	float4 mVectorTest : SV_POSITION;

	column_major float2x4 mColumnMajorMatrixTest : TEST13;
	row_major float2x4 mRowMajorMatrixTest : TEST133;

	float mFloatTest : TEST4;
	half mHalfTest : TEST5;
	
	uint mUIntTest : TEST6;
	min16uint mMin16UIntTest : TEST7;
	
	int mIntTest : TEST8;
	min16int mMin16IntTest : TEST9;
	
	unorm float mUNormTest : TEST10;
	snorm float mSNormTest : TEST11;

	bool mBoolTest : TEST12;
};

[shader("vertex")]
VSOut main(float4 Position : POSITION)
{
	VSOut ret;

	ret.mVectorTest = Position;
	ret.mFloatTest = 1.0f;
	ret.mHalfTest = 1.0;
	ret.mUIntTest = 1;
	ret.mMin16UIntTest = 2;
	ret.mIntTest = 3;
	ret.mMin16IntTest = 4;
	ret.mUNormTest = .5f;
	ret.mSNormTest = .5f;
	ret.mBoolTest = false; 
	//ret.mRowMajorMatrixTest = ret.mColumnMajorMatrixTest = matrix(ret.mVectorTest, ret.mVectorTest, ret.mVectorTest, ret.mVectorTest);

	return ret;
}