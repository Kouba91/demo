package com.kouba.shafer.service.impl;

import java.util.NoSuchElementException;
import java.util.Set;

import org.springframework.stereotype.Service;

import com.kouba.shafer.domain.User;
import com.kouba.shafer.domain.security.JwtRefreshToken;
import com.kouba.shafer.domain.security.PasswordResetToken;
import com.kouba.shafer.domain.security.UserRole;
import com.kouba.shafer.repository.JwtRefreshTokenRepository;
import com.kouba.shafer.repository.PasswordResetTokenRepository;
import com.kouba.shafer.repository.RoleRepository;
import com.kouba.shafer.repository.UserRepository;
import com.kouba.shafer.service.UserService;

@Service
public class UserServiceImpl implements UserService {
	

	private UserRepository userRepository;
	private RoleRepository roleRepository;
	private PasswordResetTokenRepository passwordResetTokenRepository;
	private JwtRefreshTokenRepository jwtRefreshTokenRepository;

	public UserServiceImpl(UserRepository userRepository, RoleRepository roleRepository, PasswordResetTokenRepository passwordResetTokenRepository,JwtRefreshTokenRepository jwtRefreshTokenRepository) {
		this.userRepository = userRepository;
		this.roleRepository = roleRepository;
		this.passwordResetTokenRepository = passwordResetTokenRepository;
		this.jwtRefreshTokenRepository = jwtRefreshTokenRepository;
	}

	@Override
	public User findByUsername(String username) {
		
		User user = userRepository.findByUsername(username);
		
		return user;
	}

	@Override
	public User save(User user) {
		User savedUser = userRepository.save(user);
		return savedUser;
	}

	@Override
	public User createUser(User user, Set<UserRole> userRoles) {
		
		for (UserRole ur : userRoles) {
			roleRepository.save(ur.getRole());
		}
			
		user.getUserRoles().addAll(userRoles);
		
		return userRepository.save(user);
	}
	
	@Override
	public PasswordResetToken getPasswordResetToken(final String token) {
		return passwordResetTokenRepository.findByToken(token);
	}

	@Override
	public void createPasswordResetTokenForUser(User user, String token) {
		final PasswordResetToken myToken = new PasswordResetToken(token, user);
		passwordResetTokenRepository.save(myToken);		
	}

	@Override
	public void deletePasswordResetToken(PasswordResetToken token) {
		passwordResetTokenRepository.delete(token);
	}

	@Override
	public User findById(Long id) {
		return userRepository.findById(id).get();
	}

	@Override
	public void createJwtRefreshTokenForUser(User user, String token) {
		JwtRefreshToken refreshToken = new JwtRefreshToken(user, token, null);
		jwtRefreshTokenRepository.save(refreshToken);		
	}

	@Override
	public void deleteJwtRefreshTokenForUser(User user) {
		try{
			JwtRefreshToken token = jwtRefreshTokenRepository.findByUserId(user.getId()).get();
			jwtRefreshTokenRepository.delete(token);
		} catch (NoSuchElementException e) {
			System.out.println("Refresh token doesnt exist");
		}
	}

	@Override
	public JwtRefreshToken findRefreshTokenByUserId(Long userId) {
		
		try {
			JwtRefreshToken token = jwtRefreshTokenRepository.findByUserId(userId).get();
			return token;
		} catch (NoSuchElementException e) {
			return null;
		}
		
	}

}
