Ashley Currie 10159991

Assignment #1 CPSC 587

The roller coaster starts below the slope of a hill. It is pulled up by a constant speed. When the roller coaster reaches the peak of the slope it is pulled down by gravity. The speed that the cart under the influence of gravity moves at can be calculated as follows v = sqrt(2g (H - h)) where v is the speed of the cart, g is gravitational acceleration (9.81 m/s^2), H is the highest point on the roller coaster, and h is the current distance of the cart from the ground. Once the rollercoaster is nearing it's end it has to smoothly come to a complete stop. Inorder for this to happen the speed at any given point during the deceleration phase will decrease by a factor of it's distance from the end of the curve divided by the distance from where it began decelerating to where the curve ends. This causes the speed to slowly decrease, because the distance from the current position to the end of the curve should be less than or equal to the distance from the point where deceleration started to the end of the curve.

To move the cart along the curve the equation s = v * t is used. This equation states that change in position is equal to speed multiplied by the change in time. For the purposes of this assignment the change in time remains constant, so the chanage in positiion is easily calculated. Once the change in position is known the cart is moved this distance along the curve. 


As the cart moves along the curve there are two forces at work keeping it on the rails. These are the gravitational and centripetal forces. Gravity pulls the cart in a vertical direction, while the centripetal force pulls it into the center of an imaginary circle that fits under the curve. Inorder to insure that the passenger of the cart does not fallout the cart must be oriented so that it's normal is exactly opposite to the direction of the centripetal force. This is done by calculating the direction of the centripetal force at the current position and rotating the cart, so that it's normal falls along the vector defining this force. This can be calculated using the formula N = (v^2/r) * n + vec3(0,g,0) where v is the current speed of the cart, r is the radius of this imaginary circle, n is the normal of the curve at the current position, and g is gravitational acceleration. The cart is also oriented in terms of two other vectors the tangent and the binormal. The tangent is simply a vector which defines the direction of the curve at a given point. The binormal is perpendicular to both the tangent and the normal. It is calculated by taking the cross product between the tangent and the normal. 

I computed the normals of my curve using the second response from http://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d

To switch between the first person view and static view press the V key.