in vec2 fragTexCoord;
out vec4 out_FragColor;
uniform sampler2D tex;

void main()
{
	out_FragColor = texture(tex, fragTexCoord);
}
